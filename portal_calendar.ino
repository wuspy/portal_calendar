#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include "config.h"
#include "Display.h"

#if !defined(AUTOMATIC_TIME_ZONE) && !defined(MANUAL_TIME_ZONE) && !defined(POSIX_TIME_ZONE)
    #error No timezone configured
#endif

// #define LED_B           18
// #define LED_G           17
// #define LED_R           16

#define uS_PER_S 1000000
#define ONE_HOUR 3600
#define SIX_HOURS ONE_HOUR * 6

/**
 * Time in seconds we'll wait for an NTP sync before giving up
 */
#define NTP_TIMEOUT 5

/**
 * Controls how long before midnight the processor is woken up to sync with NTP so it can have an accurate time for the date changeover.
 * This should be set to the maximum possible amount you expect the internal clock to be off in one day, since it will sleep for an entire day
 * and be woken at this time.
 * 
 * If the clock is running +MINUTES_BEFORE_MIDNIGHT_TO_SYNC fast per day, then in reality it will be woken up MINUTES_BEFORE_MIDNIGHT_TO_SYNC * 2
 * minutes before midnight, which is the maximum duration the NTP measurement will be deemed acceptable for. Any faster than that, and multiple
 * NTP syncs will happen per day, which wastes battery.
 * 
 * If the clock is running -MINUTES_BEFORE_MIDNIGHT_TO_SYNC slow per day, then it will wake up and sync exactly at midnight. Any slower than that,
 * and it won't wake wake up in time for midnight and the date changeover will be late.
 */
#define MINUTES_BEFORE_MIDNIGHT_TO_SYNC 30

Display display;

/**
 * Stores the current day of the year displayed
 */
RTC_DATA_ATTR int displayedYDay = 0;

enum TimezoneType: uint8_t
{
    TZT_NONE,
    TZT_POSIX,
    TZT_MANUAL,
    TZT_GEOIP,
};

/**
 * Controls if initTime() will try to sync with NTP when called.
 *
 * Can be set to true before going to sleep if a sync is desired on the next wakeup, however
 * initTime() is the only place this should ever be changed to false.
 */
RTC_DATA_ATTR bool needsNtpSync = true;

RTC_DATA_ATTR char savedTimezone[57];
RTC_DATA_ATTR TimezoneType savedTimezoneType = TZT_NONE;

void deepSleep(time_t seconds)
{
    esp_sleep_enable_timer_wakeup((uint64_t)seconds * uS_PER_S);
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

#if defined(AUTOMATIC_TIME_ZONE) || defined(MANUAL_TIME_ZONE)

/**
 * Looks up a timezone by Olson name using the timezoned.rop.nl service, which is part of the ezTime project (https://github.com/ropg/ezTime),
 * although I decided using the entirety of ezTime wasn't ideal.
 * 
 * This can also lookup timezone by IP address by passing "GeoIP", however I've found that pretty unreliable for where I live.
 * 
 * @return The POSIX timezone string, or empty string if unsuccessful
 */
String getPosixTz(String olsonOrGeoIp)
{
    WiFiUDP udp;
    udp.flush();
    udp.begin(2342);
    unsigned long started = millis();
    udp.beginPacket("timezoned.rop.nl", 2342);
    udp.write((const uint8_t*)olsonOrGeoIp.c_str(), olsonOrGeoIp.length());
    udp.endPacket();
    
    // Wait for packet or return false with timed out
    while (!udp.parsePacket()) {
        delay (1);
        if (millis() - started > 2000) {
            udp.stop();
            return "";
        }
    }
    // Stick result in String recv 
    String recv;
    recv.reserve(60);
    while (udp.available()) {
        recv += (char)udp.read();
    }
    udp.stop();
    if (recv.substring(0, 3) == "OK ") {
        return recv.substring(recv.indexOf(" ", 4) + 1);
    }
    return "";
}

#endif

#if defined(AUTOMATIC_TIME_ZONE)

/**
 * Looks up a timezone by IP address using worldtimeapi.org and timezoned.rop.nl.
 * Currently, worldtimeapi.org doesn't return a POSIX timezone string, just the Olson name, so even
 * if it's successful a call to timezoned.rop.nl is still required.
 * 
 * @return The POSIX timezone string, or empty string if unsuccessful 
 */
String getGeoIpTz()
{
    String olsonOrGeoIp = "GeoIP";
    HTTPClient http;
    http.begin("http://worldtimeapi.org/api/ip.txt");
    if (http.GET() == 200) {
        String response = http.getString();
        int startIdx = response.indexOf("timezone: ");
        if (startIdx != -1) {
            int endIdx = response.indexOf('\n', startIdx);
            if (endIdx != -1) {
                olsonOrGeoIp = response.substring(startIdx + 10, endIdx);
            }
        }
    }
    http.end();
    return getPosixTz(olsonOrGeoIp);
}

#endif

/**
 * Returns true if t (or the current time if t isn't passed) is a recent time,
 * which probably means it has been set at some point.
 */
bool isValidTime(const time_t *t = nullptr)
{
    return (t ? *t : time(nullptr)) > 1455334225;
}

/**
 * Initializes the system time and timezone, syncing with NTP if necessary.
 *
 * If NTP sync fails and the system time has never been set before, this function will sleep and won't return.
 */
void initTime()
{
    if (needsNtpSync || savedTimezoneType == TZT_NONE) {
        if (startWifi()) {
            #if defined(AUTOMATIC_TIME_ZONE) || defined(MANUAL_TIME_ZONE)
            String tz;
            #endif
            #if defined(AUTOMATIC_TIME_ZONE)
            tz = getGeoIpTz();
            if (!tz.isEmpty()) {
                strcpy(savedTimezone, tz.c_str());
                savedTimezoneType = TZT_GEOIP;
            }
            #endif
            #if defined(MANUAL_TIME_ZONE)
            if (savedTimezoneType <= TZT_MANUAL) {
                tz = getPosixTz(MANUAL_TIME_ZONE);
                if (!tz.isEmpty()) {
                    strcpy(savedTimezone, tz.c_str());
                    savedTimezoneType = TZT_MANUAL;
                }
            }
            #endif
            #if defined(POSIX_TIME_ZONE)
            if (savedTimezoneType == TZT_NONE) {
                strcpy(savedTimezone, POSIX_TIME_ZONE);
                savedTimezoneType = TZT_POSIX;
            }
            #endif

            if (savedTimezoneType == TZT_NONE) {
                stopWifi();
                display.error("Timezone lookup failed");
                deepSleep(ONE_HOUR);
            }
            bool ntpSuccessful = false;

            // Since the NTP client runs asynchronously, in order to know when it's actually
            // updated we need to set the current system time to zero, then poll and wait for it
            // to be someting much greater than zero.
            timeval oldTime;
            timeval now = { .tv_sec=0, .tv_usec=0 };
            gettimeofday(&oldTime, nullptr);
            settimeofday(&now, nullptr);
            configTzTime(savedTimezone, NTP_SERVERS);

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
                needsNtpSync = false;
            } else if (isValidTime(&oldTime.tv_sec)) {
                // Sync unsuccesful, restore old system time
                gettimeofday(&now, nullptr);
                now.tv_sec += oldTime.tv_sec + (now.tv_usec + oldTime.tv_usec) / uS_PER_S;
                now.tv_usec = (now.tv_usec + oldTime.tv_usec) % uS_PER_S;
                settimeofday(&now, nullptr);
            } else {
                // Sync unsuccesful and we have no idea what time it is
                display.error("Time synchronization failed");
                deepSleep(ONE_HOUR);
            }
        } else if (isValidTime() && savedTimezoneType != TZT_NONE) {
            // WiFi didn't connect, set timezone and keep using existing system time
            setenv("TZ", savedTimezone, 1);
            tzset();
        } else {
            // WiFi didn't connect and we have no idea what time it is
            display.error("Couldn't connect to WiFi");
            deepSleep(ONE_HOUR);
        }
    } else {
        // Set timezone
        setenv("TZ", savedTimezone, 1);
        tzset();
    }
}

time_t getSecondsToMidnight(tm *now)
{
    tm tomorrow = *now;
    ++tomorrow.tm_mday; // mktime will handle day/month rolling over
    tomorrow.tm_hour = tomorrow.tm_min = tomorrow.tm_sec = 0;
    return (time_t)difftime(mktime(&tomorrow), mktime(now));
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
        display.update(&now);
        displayedYDay = now.tm_yday;
    }

    // Go to sleep

    getLocalTime(&now); // Update time measurement
    time_t secondsToMidnight = getSecondsToMidnight(&now) + 1; // +1 to make sure it's actually at or past midnight
    if (needsNtpSync) {
        // initTime()'s attempt to sync with NTP failed
        if (secondsToMidnight <= MINUTES_BEFORE_MIDNIGHT_TO_SYNC * 60) {
            // Keep trying to sync it often since we're close to (when we think) midnight is.
            deepSleep(min(secondsToMidnight, (time_t)MINUTES_BEFORE_MIDNIGHT_TO_SYNC * 60 / 5));
        } else {
            // Keep trying to sync it every 6 hours, rather than waiting potentially another full day
            // and having the time be off by even more.
            deepSleep(min(secondsToMidnight - MINUTES_BEFORE_MIDNIGHT_TO_SYNC * 60, (time_t)SIX_HOURS));
        }
    } else if (secondsToMidnight <= MINUTES_BEFORE_MIDNIGHT_TO_SYNC * 2 * 60) {
        // Sleep until midnight
        // (this checks MINUTES_BEFORE_MIDNIGHT_TO_SYNC * 2 becase we allow the clock to be running fast by that much)
        deepSleep(secondsToMidnight);
    } else {
        // Sleep until the next NTP sync right before midnight
        needsNtpSync = true;
        deepSleep(secondsToMidnight - MINUTES_BEFORE_MIDNIGHT_TO_SYNC * 60);
    }
}

void loop()
{
}
