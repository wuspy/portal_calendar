#include <WiFi.h>
#include "global.h"
#include "time_util.h"
#include "Display.h"
#ifdef SHOW_WEATHER
#include "weather.h"
#endif

/**
 * Stores the current day of the year displayed
 */
RTC_DATA_ATTR int displayedYDay = 0;

/**
 * Controls if an NTP sync will be performed on wakeup
 */
RTC_DATA_ATTR bool needsNtpSync = true;

#ifdef TIME_ZONE
/**
 * Controls if timezone information will be synced on wakeup
 */
RTC_DATA_ATTR bool needsTimezoneSync = true;
#endif // TIME_ZONE

/**
 * Caches the current POSIX timezone string
 */
#ifdef POSIX_TIME_ZONE
    RTC_DATA_ATTR char savedTimezone[57] = POSIX_TIME_ZONE;
#else
    RTC_DATA_ATTR char savedTimezone[57] = {'\0'};
#endif // POSIX_TIME_ZONE

#ifdef SHOW_WEATHER
/**
 * Controls if weather information will be synced on wakeup
 */
RTC_DATA_ATTR bool needsWeatherSync = true;
RTC_DATA_ATTR time_t displayedWeatherVersion = 0;
#endif // SHOW_WEATHER

Display display;

void deepSleep(uint64_t seconds)
{
    DEBUG_PRINT("Sleeping for %lus", seconds);
    unsigned long start = millis(); // Stopping wifi can take time
    stopWifi();
    #ifdef ENABLE_RTC_CORRECTION
    double rtcCorrectionFactor = getRtcCorrectionFactor();
    if (rtcCorrectionFactor) {
        // Adjust the duration to sleep by the correction factor, and adjust the system time so
        // we still wake up at the time we think we will
        uint64_t adjustment = (uint64_t)round((double)seconds * rtcCorrectionFactor);
        if (adjustment) {
            timeval now;
            gettimeofday(&now, nullptr);
            now.tv_sec -= (time_t)adjustment;
            settimeofday(&now, nullptr);
            seconds += adjustment;
            DEBUG_PRINT("Sleeping for %lus (corrected)", seconds);
        }
    }
    #endif
    uint64_t duration = millis() - start;
    esp_sleep_enable_timer_wakeup(seconds * uS_PER_S - duration * 1000);
    esp_deep_sleep_start();
}

void stopWifi()
{
    if (WiFi.getMode() != WIFI_OFF) {
        DEBUG_PRINT("Stopping WiFi");
        unsigned long start = millis();
        WiFi.disconnect(true, true);
        WiFi.mode(WIFI_OFF);
        DEBUG_PRINT("WiFi shutdown took %lums", millis() - start);
    }
}

bool startWifi()
{
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }
    
    DEBUG_PRINT("Starting WiFi with hostname %s", HOSTNAME);
    WiFi.setHostname(HOSTNAME);

    unsigned long start = millis();
    #ifdef WIFI_PASS
    WiFi.begin(WIFI_NAME, WIFI_PASS);
    #else
    WiFi.begin(WIFI_NAME);
    #endif
    if (WiFi.waitForConnectResult(10000) != WL_CONNECTED) {
        DEBUG_PRINT("WiFi connection failed after %lums", millis() - start);
        stopWifi();
        return false;
    }
    DEBUG_PRINT("WiFi connection took %lums", millis() - start);
    return true;
}

void error(std::initializer_list<String> message)
{
    DEBUG_PRINT("Sleeping with error");
    stopWifi(); // Power down wifi before updating display to limit current draw from battery
    display.error(message, true);
    deepSleep(SECONDS_PER_HOUR);
}

void errorNoWifi()
{
    error({
        "NO WIFI CONNECTION",
        "",
        "Your WiFi network is either down, out of range,",
        "or you entered the wrong password.",
        "",
        "WiFi Name:",
        "\"" WIFI_NAME "\""
    });
}

void errorNtpFailed()
{
    error({
        "NO INTERNET CONNECTION",
        "",
        "Your WiFi network works, but the NTP servers didn't",
        "respond. This probably means your WiFi has no internet",
        "connection. Or, you configured the NTP servers yourself,",
        "in which case you might have messed something up."
    });
}

#ifdef TIME_ZONE

void errorTzLookupFailed()
{
    error({
        "TIMEZONE LOOKUP FAILED",
        "",
        "Your timezone is either invalid, or the timezone servers",
        "are down. If you configured the timezone servers",
        "yourself, you might have messed something up.",
        "",
        "Your timezone:",
        "\"" TIME_ZONE "\""
    });
}

#endif // TIME_ZONE

#ifdef SHOW_WEATHER

void errorInvalidOwmApiKey()
{
    error({
        "INVALID OPENWEATHERMAP API KEY",
        "",
        "OpenWeatherMap.org says your API key is invalid.",
        "You probably have an issue with your configuration.",
        "Go to your account -> My API Keys and make sure",
        "the one there matches the one you entered. Or, just",
        "disable the weather feature entirely."
    });
}

#ifdef WEATHER_LOCATION

void errorInvalidOwmLocation()
{
    error({
        "INVALID WEATHER LOCATION",
        "",
        "OpenWeatherMap.org couldn't find any results",
        "for the weather location you entered. You",
        "probably have an issue with your configuration.",
        "",
        "You Location:",
        "\"" WEATHER_LOCATION "\""
    });
}

#endif // WEATHER_LOCATION

#endif // SHOW_WEATHER

void errorBrownout()
{
    // Brownout was likely caused by the wifi radio, so hopefully there's still
    // enough power to refresh the display
    DEBUG_PRINT("Brownout detected");
    display.error({
        "REPLACE BATTERIES",
        "",
        "If the device does not restart automatically",
        "after new batteries have been inserted,",
        "press the BOOT button on the back."
    }, false);
    // Sleep forever
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    esp_deep_sleep_start();
}

int getSecondsToMidnight(tm *now)
{
    tm tomorrow = *now;
    ++tomorrow.tm_mday; // mktime will handle day/month rolling over
    tomorrow.tm_hour = tomorrow.tm_min = tomorrow.tm_sec = 0;
    return (int)difftime(mktime(&tomorrow), mktime(now));
}

void setup()
{
    time_t t;
    tm now;
    
    #ifdef DEBUG
    time(&t);
    Serial.begin(115200);
    localtime_r(&t, &now);
    char timestr[30];
    strftime(timestr, sizeof(timestr), "%d-%m-%Y %H:%M:%S", &now);
    DEBUG_PRINT("Waking up at %s", timestr);
    #endif

    if (esp_reset_reason() == ESP_RST_BROWNOUT) {
        errorBrownout();
    }

    // Set timezone

    #ifdef TIME_ZONE
    if (needsTimezoneSync || savedTimezone[0] == '\0') {
        if (startWifi()) {
            String tz = getPosixTz(TIME_ZONE);
            if (!tz.isEmpty()) {
                strcpy(savedTimezone, tz.c_str());
                needsTimezoneSync = false;
            } else if (savedTimezone[0] == '\0') {
                errorTzLookupFailed();
            }
        } else if (savedTimezone[0] == '\0') {
            // WiFi didn't connect and we have no idea what timezone we're in
            errorNoWifi();
        }
    }
    #endif // TIME_ZONE
    
    DEBUG_PRINT("Setting system timezone to %s", savedTimezone);
    setenv("TZ", savedTimezone, 1);
    tzset();

    // Set time

    time_t lastNtpSync = getLastNtpSync();
    if (needsNtpSync || !lastNtpSync) {
        bool syncMandatory = !lastNtpSync || (time_t)difftime(time(nullptr), lastNtpSync) >= ERROR_AFTER_SECONDS_WITHOUT_INTERNET;
        if (startWifi()) {
            if (syncNtp()) {
                needsNtpSync = false;
            } else if (syncMandatory) {
                // Sync unsuccesful and we have no idea what time it is
                errorNtpFailed();
            }
        } else if (syncMandatory) {
            // WiFi didn't connect and we have no idea what time it is
            errorNoWifi();
        }
    }

    time(&t);
    localtime_r(&t, &now);

    #ifdef SHOW_WEATHER

    // Sync weather

    if (needsWeatherSync && startWifi()) {
        OwmResult result = refreshWeather();
        switch (result) {
            case OwmResult::SUCCESS:
                needsWeatherSync = false;
                break;
            case OwmResult::INVALID_API_KEY:
                errorInvalidOwmApiKey();
            case OwmResult::INVALID_LOCATION:
                errorInvalidOwmLocation();
            default:
                // Ignore other OWM errors because weather isn't critical
                break;
        }
    } // Critical WiFi connection errors will be handled by the NTP/timezone syncs

    #endif

    stopWifi(); // Power down wifi before updating display to limit current draw from battery

    // Update display if needed
    bool needsDisplayUpdate = now.tm_yday != displayedYDay;
    #ifdef SHOW_WEATHER
    needsDisplayUpdate |= displayedWeatherVersion != getLastWeatherSync() && getSecondsToMidnight(&now) > SECONDS_BEFORE_MIDNIGHT_TO_SYNC_1 * 2;
    #endif
    if (needsDisplayUpdate) {
        DEBUG_PRINT("Updating display for %d-%d-%d", now.tm_year + 1900, now.tm_mon + 1, now.tm_mday);
        display.update(&now);
        displayedYDay = now.tm_yday;
        #ifdef SHOW_WEATHER
        displayedWeatherVersion = getLastWeatherSync();
        #endif
    }

    // Go to sleep

    time(&t); // Update time measurement
    localtime_r(&t, &now);
    int secondsToMidnight = getSecondsToMidnight(&now) + 1; // +1 to make sure it's actually at or past midnight
    // syncFailed doesn't care about timezone sync because as long as we've got it once it's probably still correct
    bool syncFailed = needsNtpSync;
    #ifdef SHOW_WEATHER
    syncFailed |= needsWeatherSync;
    #endif
    if (syncFailed) {
        DEBUG_PRINT("Sync failed, will retry");
        if (secondsToMidnight <= SECONDS_BEFORE_MIDNIGHT_TO_SYNC_2) {
            // Try to sync 3 more times before midnight
            deepSleep(min(secondsToMidnight, SECONDS_BEFORE_MIDNIGHT_TO_SYNC_2 / 3));
        } else if (secondsToMidnight <= SECONDS_BEFORE_MIDNIGHT_TO_SYNC_1) {
            // Try to sync 3 more times before the next day's 1st sync is scheduled
            deepSleep(min(secondsToMidnight - SECONDS_BEFORE_MIDNIGHT_TO_SYNC_2, (SECONDS_BEFORE_MIDNIGHT_TO_SYNC_1 - SECONDS_BEFORE_MIDNIGHT_TO_SYNC_2) / 3));
        } else {
            // Keep trying to sync every hour
            deepSleep(min(secondsToMidnight - SECONDS_BEFORE_MIDNIGHT_TO_SYNC_1, 3600));
        }
    } else if (secondsToMidnight <= SECONDS_BEFORE_MIDNIGHT_TO_SYNC_2 * 2) {
        // Sleep until midnight
        DEBUG_PRINT("Sleeping until midnight");
        deepSleep(secondsToMidnight);
    } else if (secondsToMidnight <= SECONDS_BEFORE_MIDNIGHT_TO_SYNC_1 * 2) {
        // Sleep until second NTP sync
        needsNtpSync = true;
        DEBUG_PRINT("Sleeping for 2nd NTP sync");
        deepSleep(secondsToMidnight - SECONDS_BEFORE_MIDNIGHT_TO_SYNC_2);
    } else {
        // Sleep until first NTP sync
        needsNtpSync = true;
        #ifdef TIME_ZONE
        needsTimezoneSync = true;
        #endif
        #ifdef SHOW_WEATHER
        needsWeatherSync = true;
        #endif
        DEBUG_PRINT("Sleeping for 1st sync");
        deepSleep(secondsToMidnight - SECONDS_BEFORE_MIDNIGHT_TO_SYNC_1);
    }
}

void loop()
{
}
