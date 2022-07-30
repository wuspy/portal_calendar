#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include "config.h"
#include "display_GDEW075T7.h"
#include "resource/font/Univers 65 Bold Regular_28px.h"
#include "resource/font/Univers LT 49 Light Ultra Condensed_400px.h"
#include "resource/aperture_logo.h"
#include "resource/progress_bar.h"
#include "resource/icons.h"
#include "resource/epd-portrait-test.h"

// #define LED_B           18
// #define LED_G           17
// #define LED_R           16

#define uS_PER_S 1000000
#define TEN_MINUTES 600
#define ONE_HOUR 3600
#define SIX_HOURS ONE_HOUR * 6

// Time in seconds we'll wait for an NTP sync before giving up
#define NTP_TIMEOUT 5

// In normal operation, NTP is synced onec per day this many minutes (+/- however much the internal clock drifts in one day) before midnight.
// This allows the time to be relatively accurate for the changeover, while also allowing the internal clock to be off by +/- this amount.
#define MINUTES_BEFORE_MIDNIGHT_TO_SYNC 50

// Controls how often initTime() will try to sync with NTP upon wakeup.
//
// Should not be less than MINUTES_BEFORE_MIDNIGHT_TO_SYNC * 2 as that could cause multiple NTP syncs per day if the internal
// clock is running slow, which wastes battery.
//
// Should also not be too long, as that could cause inaccurate date changeover on the first day after initial poweron.
#define NTP_STALE_AFTER_MINUTES MINUTES_BEFORE_MIDNIGHT_TO_SYNC * 2

#define SMALL_FONT FONT_UNIVERS_65_BOLD_REGULAR_28PX
#define LARGE_NUMBER_FONT FONT_UNIVERS_LT_49_LIGHT_ULTRA_CONDENSED_400PX

const char* MONTHS[] = {
    "JANUARY",
    "FEBRUARY",
    "MARCH",
    "APRIL",
    "MAY",
    "JUNE",
    "JULY",
    "AUGUST",
    "SEPTEMBER",
    "OCTOBER",
    "NOVEMBER",
    "DECEMBER",
};

const char* DAYS[] = {
    "SUNDAY",
    "MONDAY",
    "TUESDAY",
    "WEDNESDAY",
    "THURSDAY",
    "FRIDAY",
    "SATURDAY",
};

Display *display = nullptr;

// Stores the last time the clock was synced with NTP
RTC_DATA_ATTR time_t lastTimeSync = 0;
// Stores the current day of the year displayed
RTC_DATA_ATTR int displayedYDay = 0;

void initDisplay()
{
    if (display == nullptr) {
        display = new Display(SPI_BUS, CS_PIN, RESET_PIN, DC_PIN, BUSY_PIN);
        display->setRotation(Display::ROTATION_90);
        display->setAlpha(Display::WHITE);
    }
}

void updateCalendarDisplay(const tm *now)
{
    initDisplay();
    int daysInMonth = getDaysInMonth(now->tm_mon + 1, now->tm_year + 1900);

    // Static lines
    display->drawHLine(110, 50, 360, 2, Display::BLACK, Display::TOP_LEFT);
    display->drawHLine(110, 430, 360, 2, Display::BLACK, Display::TOP_LEFT);
    display->drawHLine(110, 538, 360, 2, Display::BLACK, Display::TOP_LEFT);

    // Aperture logo
    display->drawImage(IMG_APERTURE_LOGO, 110, 740);

    // BIG date
    char bigDay[3];
    sprintf(bigDay, "%02d", now->tm_mday);
    display->drawText(bigDay, LARGE_NUMBER_FONT, 110, 16, 10);

    // Small "XX/XX" date
    char smallDay[6];
    sprintf(smallDay, "%02d/%02d", now->tm_mday, daysInMonth);
    display->drawText(smallDay, SMALL_FONT, 110, 394);

    #ifdef SHOW_DAY
    // Day name
    display->drawText(DAYS[now->tm_wday], SMALL_FONT, 469, 394, 0, Display::TOP_RIGHT);
    #endif

    #ifdef SHOW_MONTH
    // Month name
    display->drawText(MONTHS[now->tm_mon], SMALL_FONT, 110, 14, 0);
    #endif

    #ifdef SHOW_YEAR
    // Year
    char year[5];
    sprintf(year, "%d", now->tm_year + 1900);
    display->drawText(year, SMALL_FONT, 469, 14, 0, Display::TOP_RIGHT);
    #endif

    // Progress bar
    display->drawImage(IMG_PROGRESS_BAR, 110, 438);
    int32_t progressWidth = IMG_PROGRESS_BAR.width * now->tm_mday / daysInMonth;
    display->fillRect(110 + progressWidth, 438, IMG_PROGRESS_BAR.width - progressWidth, IMG_PROGRESS_BAR.height, Display::WHITE);

    // Icons, temporary for now
    display->drawImage(IMG_ICONS, 110, 550);
    
    display->refresh();
}

void displayError(String str)
{
    initDisplay();
    display->drawText(
        str,
        SMALL_FONT,
        display->getWidth() / 2,
        display->getHeight() / 2,
        0,
        Display::CENTER
    );
    display->refresh();
}

void deepSleep(time_t seconds)
{
    auto result = esp_sleep_enable_timer_wakeup((uint64_t)seconds * uS_PER_S);
    esp_deep_sleep_start();
}

void stopWifi()
{
    if (WiFi.getMode() != WIFI_OFF) {
        WiFi.disconnect(true, true);
        WiFi.mode(WIFI_OFF);
    }
}

bool startWifi()
{
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }
    WiFi.begin(WIFI_NAME, WIFI_PASS);
    if (WiFi.waitForConnectResult(10000) != WL_CONNECTED) {
        stopWifi();
        return false;
    }
    return true;
}

/**
 * Returns true if t (or the current time if t isn't passed) is a recent time
 * which probably means it has been set at some point.
 */
bool isValidTime(const time_t *t = nullptr)
{
    return (t ? *t : time(nullptr)) > 1455334225;
}

bool needsNtpUpdate()
{
    time_t now = time(nullptr);
    return !isValidTime(&now) || now - lastTimeSync >= NTP_STALE_AFTER_MINUTES * 60;
}

/**
 * Initializes the system time and timezone, syncing with NTP if necessary.
 *
 * If NTP sync fails and the system time has never been set before, this function will sleep and won't return.
 */
void initTime()
{
    setenv("TZ", POSIX_TIME_ZONE, 1);
    tzset();
    if (needsNtpUpdate()) {
        if (startWifi()) {
            bool ntpSuccessful = false;

            // Since the NTP client runs asynchronously, in order to know when it's actually
            // updated we need to set the current system time to zero, then poll and wait for it
            // to be someting much greater than zero.
            timeval oldTime;
            timeval now = { .tv_sec=0, .tv_usec=0 };
            gettimeofday(&oldTime, nullptr);
            settimeofday(&now, nullptr);
            configTzTime(POSIX_TIME_ZONE, NTP_SERVERS);

            // This loop is pretty similar to what getLocalTime in esp32-hal-time does, but it
            // can be simplified for our purpose
            do {
                delay(10);
                gettimeofday(&now, nullptr);
                if (isValidTime(&now.tv_sec)) {
                    ntpSuccessful = true;
                    break;
                }
            } while (now.tv_sec < NTP_TIMEOUT);

            stopWifi();
            if (ntpSuccessful) {
                time(&lastTimeSync);
            } else if (isValidTime(&oldTime.tv_sec)) {
                // Sync unsuccesful, restore old system time
                gettimeofday(&now, nullptr);
                now.tv_sec += oldTime.tv_sec + (now.tv_usec + oldTime.tv_usec) / uS_PER_S;
                now.tv_usec = (now.tv_usec + oldTime.tv_usec) % uS_PER_S;
                settimeofday(&now, nullptr);
            } else {
                // Sync unsuccesful and we have no idea what time it is
                displayError("Time synchronization failed");
                deepSleep(ONE_HOUR);
            }
        } else if (!isValidTime()) {
            // WiFi didn't connect and we have no idea what time it is
            displayError("Couldn't connect to WiFi");
            deepSleep(ONE_HOUR);
        }
    }
}

time_t getSecondsToMidnight(tm *now)
{
    tm tomorrow = *now;
    ++tomorrow.tm_mday; // mktime will handle day/month rolling over
    tomorrow.tm_hour = tomorrow.tm_min = tomorrow.tm_sec = 0;
    return (time_t)difftime(mktime(&tomorrow), mktime(now));
}

int getDaysInMonth(int month, int year)
{
    switch (month) {
        case 2:
            return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0) ? 29 : 28;
        case 4:
        case 6:
        case 9:
        case 11:
            return 30;
        default:
            return 31;
    }
}

void setup()
{
    // pinMode(LED_B, OUTPUT);
    // pinMode(LED_G, OUTPUT);
    // pinMode(LED_R, OUTPUT);
    // digitalWrite(LED_B, HIGH);
    // digitalWrite(LED_G, HIGH);
    // digitalWrite(LED_R, HIGH);

    // Figure out what time it is

    initTime();
    tm now;
    getLocalTime(&now);

    // Update display if needed
    
    if (now.tm_yday != displayedYDay) {
        updateCalendarDisplay(&now);
        displayedYDay = now.tm_yday;
    }

    // Go to sleep

    getLocalTime(&now);
    time_t secondsToMidnight = getSecondsToMidnight(&now);
    if (secondsToMidnight <= MINUTES_BEFORE_MIDNIGHT_TO_SYNC * 60) {
        if (needsNtpUpdate()) {
            // initTime()'s attempt to sync with NTP failed.
            // Keep trying to sync it every 10 minutes since we're close to (when we think) midnight is.
            deepSleep(min(secondsToMidnight, (time_t)TEN_MINUTES));
        } else {
            // Sleep until midnight
            deepSleep(secondsToMidnight);
        }
    } else {
        if (needsNtpUpdate()) {
            // initTime()'s attempt to sync with NTP failed.
            // Keep trying to sync it every 6 hours, rather than waiting potentially another full day
            // and having the time be off by even more.
            deepSleep(min(secondsToMidnight - MINUTES_BEFORE_MIDNIGHT_TO_SYNC * 60, (time_t)SIX_HOURS));
        } else {
            // Sleep the rest of the day
            deepSleep(secondsToMidnight - MINUTES_BEFORE_MIDNIGHT_TO_SYNC * 60);
        }
    }
}

void loop()
{
}
