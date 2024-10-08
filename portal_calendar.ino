#include <Arduino.h>
#include <WiFi.h>
#include <math.h>
#include "global.h"
#include "Configuration.h"
#include "time_util.h"
#include "Display.h"
#include "driver/rtc_io.h"
#include "weather.h"

const uint8_t ACTION_NTP_SYNC = 0b001;
const uint8_t ACTION_TZ_SYNC = 0b010;
const uint8_t ACTION_WEATHER_SYNC = 0b100;

/**
 * A bitmask of the actions above. Controls what is scheduled to be performed on scheduledWakeup.
 */
RTC_DATA_ATTR uint8_t scheduledActions = 0;
/**
 * The next scheduled wakeup time.
 */
RTC_DATA_ATTR time_t scheduledWakeup = 0;
/**
 * Time the current deep sleep cycle was started. Needed for RTC drift correction.
 */
RTC_DATA_ATTR time_t sleepStartTime = 0;

/**
 * Stores the current day of the year displayed
 */
RTC_DATA_ATTR int displayedYDay = 0;

RTC_DATA_ATTR bool showWeather = false;
RTC_DATA_ATTR time_t displayedWeatherTime = 0;

void deepSleep(time_t seconds)
{
    time(&sleepStartTime);
    scheduledWakeup = sleepStartTime + seconds;
    correctSleepDuration(&seconds);
    log_i("Sleeping for %llds, scheduled wakeup is %s\n\n", (long long)seconds, printTime(scheduledWakeup));

    if (Config.getWeatherEnabled()) {
        // Enable wakeup on boot button
        #ifdef ENABLE_GPIO0_PULLUP
        esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
        rtc_gpio_pullup_en(GPIO_NUM_0);
        rtc_gpio_pulldown_dis(GPIO_NUM_0);
        #endif // ENABLE_GPIO0_PULLUP
        esp_sleep_enable_ext1_wakeup(0x1 << GPIO_NUM_0, ESP_EXT1_WAKEUP_ALL_LOW);
    }

    // Enable timer wakeup
    esp_sleep_enable_timer_wakeup((uint64_t)seconds * uS_PER_S);
    esp_deep_sleep_start();
}

void stopWifi()
{
    if (WiFi.getMode() != WIFI_OFF) {
        log_i("Stopping Wi-Fi");
        unsigned long start = millis();
        WiFi.disconnect(true, true);
        WiFi.mode(WIFI_OFF);
        log_i("Wi-Fi shutdown took %lums", millis() - start);
    }
}

void error(String message)
{
    log_i("Sleeping with error");
    stopWifi(); // Power down wifi before updating display to limit current draw from battery
    Display.error(message, true);
    deepSleep(SECONDS_PER_HOUR);
}

void errorNoWifi()
{
    error(
        "NO WI-FI CONNECTION\n\n"
        "Your Wi-Fi network is either down, out of range, "
        "or you entered the wrong password.\n\n"
        "Wi-Fi Name:\n"
        + Config.getWifiSsid()
    );
}

void errorNtpFailed()
{
    error(
        "NO INTERNET CONNECTION\n\n"
        "Your Wi-Fi network works, but the NTP servers didn't "
        "respond. This probably means your Wi-Fi has no internet "
        "connection. Or, you configured the NTP servers yourself, "
        "in which case you might have messed something up.\n\n"
        "NTP servers:\n"
        + Config.getPrimaryNtpServer() + "\n"
        + Config.getSecondaryNtpServer()
    );
}

void errorTzLookupFailed()
{
    error(
        "TIMEZONE LOOKUP FAILED\n\n"
        "Your timezone is either invalid, or the timezone servers "
        "are down. If you configured the timezone servers "
        "yourself, you might have messed something up.\n\n"
        "Your timezone:\n"
        + Config.getTimezoneName() + "\n\n"
        "Timezone servers:\n"
        + Config.getPrimaryTimezonedServer() + "\n"
        + Config.getSecondaryTimezonedServer()
    );
}

void errorInvalidOwmApiKey()
{
    error(
        "INVALID OPENWEATHERMAP API KEY\n\n"
        "OpenWeatherMap.org says your API key is invalid. "
        "You probably have an issue with your configuration. "
        "Go to your account -> My API Keys and make sure "
        "the one there matches the one you entered. Or, just "
        "disable the weather feature entirely."
    );
}

void errorBrownout()
{
    // Brownout was likely caused by the wifi radio, so hopefully there's still
    // enough power to refresh the display
    log_w("Brownout detected");
    Display.error(
        "REPLACE BATTERIES\n\n"
        "If the device does not restart automatically "
        "after new batteries have been inserted, "
        "press the RESET button on the back.",
        false
    );
    // Sleep forever
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    esp_deep_sleep_start();
}

void startConfigServer()
{
    Config.startConfigServer();
    if (Config.wasSaved()) {
        // Reset state
        scheduledWakeup = 0;
        scheduledActions = 0;
        savedTimezone[0] = '\0';
        showWeather = Config.getWeatherEnabled();
        displayedYDay = 0;
        displayedWeatherTime = 0;
        lastWeatherSync = 0;
    }
}

void showWelcomeScreen()
{
    Display.showWelcomeScreen();
    // Sleep forever
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    esp_deep_sleep_start();
}

void runScheduledActions()
{
    // Sync timezone

    if ((scheduledActions & ACTION_TZ_SYNC) || !isSystemTimeValid()) {
        if (Config.connectToSavedWifi()) {
            String tz;
            TimezonedResult result = getPosixTz(
                { Config.getPrimaryTimezonedServer(), Config.getSecondaryTimezonedServer() },
                Config.getTimezoneName(),
                tz
            );
            if (result == TimezonedResult::Ok) {
                setTimezone(tz.c_str());
                scheduledActions &= ~ACTION_TZ_SYNC;
            } else if (!savedTimezone[0]) {
                errorTzLookupFailed();
            }
        } else if (!savedTimezone[0]) {
            // WiFi didn't connect and we have no idea what timezone we're in
            errorNoWifi();
        }
    }

    // Sync time

    if ((scheduledActions & ACTION_NTP_SYNC) || !lastNtpSync) {
        if (Config.connectToSavedWifi()) {
            if (syncNtp({ Config.getPrimaryNtpServer(), Config.getSecondaryNtpServer() })) {
                scheduledActions &= ~ACTION_NTP_SYNC;
            } else if (!isSystemTimeValid()) {
                // Sync unsuccesful and we have no idea what time it is
                errorNtpFailed();
            }
        } else if (!isSystemTimeValid()) {
            // WiFi didn't connect and we have no idea what time it is
            errorNoWifi();
        }
    }

    // Sync weather
    
    if ((scheduledActions & ACTION_WEATHER_SYNC) || !lastWeatherSync) {
        if (Config.getWeatherEnabled()) {
            if (Config.connectToSavedWifi()) {
                OwmResult result = refreshWeather();
                switch (result) {
                    case OwmResult::SUCCESS:
                        scheduledActions &= ~ACTION_WEATHER_SYNC;
                        break;
                    case OwmResult::INVALID_API_KEY:
                        errorInvalidOwmApiKey();
                    default:
                        // Ignore other OWM errors because weather isn't critical
                        break;
                }
            } else if (!lastWeatherSync) {
                errorNoWifi();
            }
        } else {
            scheduledActions &= ~ACTION_WEATHER_SYNC;
        }
    }
}

void setup()
{
    time_t t;
    time(&t);

    #if CORE_DEBUG_LEVEL > 0
    Serial.begin(115200);
    #endif

    setTimezone();
    if (sleepStartTime) {
        correctSystemClock(t - sleepStartTime);
        time(&t);
        sleepStartTime = 0;
    }

    log_i("Waking up at %s", printTime(t));

    const esp_reset_reason_t resetReason = esp_reset_reason();
    const esp_sleep_wakeup_cause_t wakeupCause = esp_sleep_get_wakeup_cause();

    log_i("Wakeup cause: %u, Reset reason: %u", wakeupCause, resetReason);

    if (resetReason == ESP_RST_BROWNOUT) {
        errorBrownout();
    }

    Config.begin();

    // Check if configuration is required
    if (!Config.isConfigured()) {
        log_i("Not configured");
        while (!Config.isConfigured()) {
            if (Config.isOnUsbPower()) {
                log_i("On USB power");
                startConfigServer();
            } else {
                log_i("Not on USB power");
                showWelcomeScreen();
            }
        }
    }
    // Check for wakeup from boot button press
    else if (wakeupCause == ESP_SLEEP_WAKEUP_EXT1 && Config.getWeatherEnabled() && isSystemTimeValid()) {
        log_i("Toggling showWeather");
        showWeather = !showWeather;
    }
    // Check if config server is manually started by pressing reset while plugged in
    else if (wakeupCause == ESP_SLEEP_WAKEUP_UNDEFINED && Config.isOnUsbPower()) {    
        #ifndef DISABLE_MANUAL_CONFIG_SERVER_ACTIVATION
        log_i("Config server was manually started");
        startConfigServer();
        if (!Config.isConfigured()) {
            // Just in case they cleared some required settings
            showWelcomeScreen();
        }
        #else
        log_i("Config server would have been manually started, but is disabled");
        #endif
    }

    time(&t);

    if (t >= scheduledWakeup) {
        runScheduledActions();
        scheduledWakeup = 0;
    }

    stopWifi();
    WiFi.setHostname(Config.getHostname().c_str());

    time(&t);
    tm now;
    localtime_r(&t, &now);

    // Check if display should be updated

    bool needsDisplayUpdate = now.tm_yday != displayedYDay;

    if (Config.getWeatherEnabled()) {
        needsDisplayUpdate = needsDisplayUpdate
            || static_cast<bool>(displayedWeatherTime) != showWeather
            || (showWeather
                && lastWeatherSync != displayedWeatherTime
                && getSecondsToMidnight(&now) > SECONDS_BEFORE_MIDNIGHT_TO_SYNC_1 * 2);
    }

    if (needsDisplayUpdate) {
        log_i("Updating display at %s", printTime(t));
        Display.update(&now, getLocale(Config.getLocale()), showWeather);
        displayedYDay = now.tm_yday;
        displayedWeatherTime = Config.getWeatherEnabled() && showWeather ? lastWeatherSync : 0;
    }

    time(&t);

    if (scheduledWakeup) {
        log_i("Wakeup is already scheduled");
        // max with 0 is needed in case the display was updated since the first scheduledWakeup check,
        // which could make it in the past now despite being in the future before the display update
        deepSleep(max((time_t)0, scheduledWakeup - t));
    }

    // Schedule next wakeup

    localtime_r(&t, &now);
    int secondsToMidnight = getSecondsToMidnight(&now) + 1; // +1 to make sure it's actually at or past midnight
    log_i("%d seconds to midnight", secondsToMidnight);
    // failedActions doesn't care about timezone sync because as long as we've got it once it's probably still correct
    uint8_t failedActions = scheduledActions & ~ACTION_TZ_SYNC;
    if (failedActions) {
        log_w("Sync failed, scheduling retry");
        if (secondsToMidnight <= SECONDS_BEFORE_MIDNIGHT_TO_SYNC_1) {
            if (Config.getTwoNtpSyncsPerDay()) {
                if (secondsToMidnight <= SECONDS_BEFORE_MIDNIGHT_TO_SYNC_2) {
                    // Try to sync 3 more times before midnight
                    deepSleep(min(secondsToMidnight, SECONDS_BEFORE_MIDNIGHT_TO_SYNC_2 / 3));
                }
                // Try to sync 3 more times before the 2nd scheduled sync
                deepSleep(min(
                    secondsToMidnight - SECONDS_BEFORE_MIDNIGHT_TO_SYNC_2,
                    (SECONDS_BEFORE_MIDNIGHT_TO_SYNC_1 - SECONDS_BEFORE_MIDNIGHT_TO_SYNC_2) / 3
                ));
            }
            // Try to sync 4 more times before midnight
            deepSleep(min(secondsToMidnight, SECONDS_BEFORE_MIDNIGHT_TO_SYNC_1 / 4));
        }
        // Keep trying to sync every hour
        deepSleep(min(secondsToMidnight - SECONDS_BEFORE_MIDNIGHT_TO_SYNC_1, 3600));
    }

    if (secondsToMidnight <= SECONDS_BEFORE_MIDNIGHT_TO_SYNC_1 * 2) {
        if (!Config.getTwoNtpSyncsPerDay() || secondsToMidnight <= SECONDS_BEFORE_MIDNIGHT_TO_SYNC_2 * 2) {
            // Sleep until midnight
            log_i("Sleeping until midnight");
            deepSleep(secondsToMidnight);
        }
        // Sleep until second NTP sync
        scheduledActions = ACTION_NTP_SYNC;
        log_i("Sleeping for 2nd NTP sync");
        deepSleep(secondsToMidnight - SECONDS_BEFORE_MIDNIGHT_TO_SYNC_2);
    }

    // Sleep until first NTP sync
    scheduledActions =
        ACTION_NTP_SYNC
        | ACTION_TZ_SYNC
        | (Config.getWeatherEnabled() ? ACTION_WEATHER_SYNC : 0);
    log_i("Sleeping for 1st sync");
    deepSleep(secondsToMidnight - SECONDS_BEFORE_MIDNIGHT_TO_SYNC_1);
}

void loop()
{
}
