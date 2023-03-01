#include <WiFi.h>
#include <WiFiManager.h>  
#include "global.h"
#include "time_util.h"
#include "Display.h"
#include "weather.h"
#include "qrcodegen/qrcodegen.h"
#include "userConfig.h"

/**
 * On first boot we show the WiFi Configuration screen
 * (until it times out), but subsequent wakeups we skip.
 */
RTC_DATA_ATTR int bootCount = 0;

/**
 * Stores the current day of the year displayed
 */
RTC_DATA_ATTR int displayedYDay = 0;

/**
 * Controls if an NTP sync will be performed on wakeup
 */
RTC_DATA_ATTR bool needsNtpSync = true;

/**
 * Controls if timezone information will be synced on wakeup
 */
RTC_DATA_ATTR bool needsTimezoneSync = true;

/**
 * Controls if weather information will be synced on wakeup
 */
RTC_DATA_ATTR bool needsWeatherSync = true;
RTC_DATA_ATTR time_t lastWeatherSyncTime = 0;

// Config Save & Load (to persist between battery replacements)
UserConfig userConfig;

Display display(&userConfig);
WiFiManager wifiManager;


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
        wifiManager.disconnect();
        
        // Disable the WiFi hardware as well for power preservation.
        WiFi.mode(WIFI_OFF);
        DEBUG_PRINT("WiFi shutdown took %lums", millis() - start);
    }
}

bool startWifi()
{
  // Ensure the device is in Station Mode
  WiFi.mode(WIFI_STA);

  // If we're already connected, don't bother trying to connect again
  if(wifiManager.getLastConxResult() == WL_CONNECTED) {
    return true;
  }

  DEBUG_PRINT("Starting WiFi");
  unsigned long start = millis();
  
  // If we're not already connected, attempt to connect. By default WiFiManager will automatically
  // try to start an Access Point if connection fails (so you can connect to WiFiManager and configure)
  // but we suppress that behavior, so that we can display some info to the user.
  wifiManager.setEnableConfigPortal(false);
  wifiManager.setWiFiAutoReconnect(false); 
  wifiManager.setConnectTimeout(10); // Attempt to connect for ten seconds, then fail.

  // If there's no credentails to try and connect with, don't bother with the connection attempt.
  bool result = wifiManager.getWiFiIsSaved();
  if(!result) {
    DEBUG_PRINT("No WiFi Credentials Saved, moving user to AP Setup Flow");

    // We call stopWiFi to clean up and shut down WiFi as much as possible
    stopWifi();
    return false;
  }

  // If they do have credentials saved, attempt a connection.
  result = wifiManager.autoConnect();
  if(!result) {
    DEBUG_PRINT("WiFi connection failed after %lums, moving user to AP Setup Flow", millis() - start);

    stopWifi();
    return false;
  }

  // If we got this far then they managed to connect to WiFi!
  DEBUG_PRINT("WiFi connection succeeded, took %lums", millis() - start);
  return true;
}

void doDeviceConfigurationFlow()
{
    // To configure the device, the ESP32 will create an Access Point with a username/password.
    // The user will then connect to it via their phone, setup the access to the real WiFi
    // and then the ESP32 will close the Access Point and attempt to connect to the user-specified one.
    
    // We want to inform the user how to configure their Aperture Science Non-Sentient Device,
    // so we will take advantage of the screen to display some info + a QR Code to scan with the phone.
    // We can't use the standard error(...) function as that will enter deep sleep and prevent us from
    // ever starting the webportal.
    DEBUG_PRINT("Starting Device Configuration Flow");
    unsigned long start = millis();

    // Configure the Wifi Manager with custom fields for the various Calendar settings.
    //setupUserParametersOnAP();

    // ToDo: Find out if the ESP is WPA2 or WEP when in AP mode
    // To generate a QR Code that helps auto-connect your phone to a WiFi network, the
    // QR Code needs to have the following pattern, where H: means "Hidden" and is optional.
    // The trailing ";;" is important. Side note: This could probably be rewritten to cause
    // less heap fragmentation, since this info is the hard-coded Device AP, not user AP.
    // WIFI:S:<SSID>;T:<WEP|WPA|blank>;P:<PASSWORD>;H:<true|false|blank>;;
    String QRCodeData = String("WIFI:S:") + WIFI_AP_NAME + String(";T:WPA;P:") + WIFI_AP_PASSWORD + String(";H:false;;");

    // Now that we have the data as a string, we will generate a qr code.
    enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_LOW;  // Error correction level

    uint8_t qrcode[qrcodegen_BUFFER_LEN_FOR_VERSION(MAX_QRCODE_VERSION)];
    uint8_t tempBuffer[qrcodegen_BUFFER_LEN_FOR_VERSION(MAX_QRCODE_VERSION)];

    bool bOk = qrcodegen_encodeText(QRCodeData.c_str(), tempBuffer, qrcode, errCorLvl,
    qrcodegen_VERSION_MIN, MAX_QRCODE_VERSION, qrcodegen_Mask_AUTO, true);

    DEBUG_PRINT("QRCode Version: %d Length: %d", MAX_QRCODE_VERSION, qrcodegen_BUFFER_LEN_FOR_VERSION(MAX_QRCODE_VERSION));

    std::initializer_list<String> userText = 
    {
      "Welcome to your Aperture Science Non-Sentient Device.",
      "Please configure your device by using another Wi-Fi",
      "capable device (such as your phone) and connecting",
      "to the Wi-Fi network:",
      "",
      "SSID: \""WIFI_AP_NAME"\"",
      "Password: \""WIFI_AP_PASSWORD"\"",
      "",
      "Alternatively, you can scan the QR code above to be",
      "automatically connected. This will automatically time",
      "out and attempt to connect to the last known Wi-Fi",
      "after 10 minutes. You can bypass this now by",
      "connecting via the QR code and hitting save."
    };

    // Draw our QR Code + User Prompt to Display
    display.showQRCodeSetup(qrcode, userText);

    userConfig.addParamsToWiFiManager(wifiManager);

    // Put the device in Access Point mode for ten minutes to allow
    // the user to configure the settings. After ten minutes (or the
    // user hitting Save in the config) it will boot. This only happens
    // on first boot.)
    wifiManager.setConfigPortalTimeout(60 * 10);
    wifiManager.startConfigPortal(WIFI_AP_NAME, WIFI_AP_PASSWORD);
    DEBUG_PRINT("User has finished Config Portal configuration.");
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
  // ToDo: This needs to be converted to a dynamic string because it's the access point
  // from the WiFI manager. But we don't have vector<String> support on Arduino...
    error({
        "NO WIFI CONNECTION",
        "",
        "Your WiFi network is either down, out of range,",
        "or you entered the wrong password. Press \"RESET\"",
        "on the back of the device and check your Wi-Fi",
        "credentials through the QR Code."
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

void errorTzLookupFailed()
{
    // ToDo: Needs dynamic strings...
    error({
        "TIMEZONE LOOKUP FAILED",
        "",
        "Your timezone is either invalid, or the timezone servers",
        "are down. If you configured the timezone servers",
        "yourself, you might have messed something up.",
        //"",
        //"Your timezone:",
        //"\"" TIME_ZONE "\""
    });
}

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

void errorInvalidOwmLocation()
{
  // ToDo: Needs dynamic strings...
    error({
        "INVALID WEATHER LOCATION",
        "",
        "OpenWeatherMap.org couldn't find any results",
        "for the weather location you entered. You",
        "probably have an issue with your configuration.",
        //"",
        //"You Location:",
        //"\"" WEATHER_LOCATION "\""
    });
}

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

void saveConfigCallback()
{
  DEBUG_PRINT("Got callback from WifiManager to save config");
  userConfig.saveParamsFromWiFiManager();
}

void setup()
{
    time_t t;
    tm now;

    bootCount++;

    // Make the WiFi Manager save your settings even if the credentials aren't set/correct
    wifiManager.setBreakAfterConfig(true);
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    #ifdef DEBUG
    time(&t);

    // wifiManager.setDebugOutput(true);
    // wifiManager.resetSettings();

    // Convert timestamp and print it over Serial
    Serial.begin(115200);
    localtime_r(&t, &now);
    char timestr[30];
    strftime(timestr, sizeof(timestr), "%d-%m-%Y %H:%M:%S", &now);
    DEBUG_PRINT("Waking up at %s", timestr);
    #endif

    // If we're rebooting because we ran out of power, print an error
    // (and this function never returns, the rest of setup is not run)
    if (esp_reset_reason() == ESP_RST_BROWNOUT) {
        errorBrownout();
    }

    // Attempt to load the config file. This can return false if
    // the config file hasn't been set up before.
    userConfig.loadFromFilesystem();
    userConfig.createWiFiParams();

    // Attempt to connect to the WiFi. If connection fails then either credentials are wrong,
    // it's never been set up, or it's out of range. 
    bool bConnectedWiFi = startWifi();

    if(!bConnectedWiFi || bootCount == 1)
    {
        // Start an Access Point that the user
        // can connect to with their phone to configure this display. This will print
        // some info to the screen
        doDeviceConfigurationFlow();

        // If we've made it to here then the user has finished connecting it to a
        // real Access Point, and we can move onto time synchronization.
    }

    // Set timezone
    if(userConfig.timeZone.length() > 0)
    {
      // Attempt to convert a human-readable TZ-database string (America/Chicago) into a POSIX specification (CST6CDT,M3.2.0,M11.1.0)
      if (needsTimezoneSync || userConfig.posixTimeZone.length() == 0) {
        if (startWifi()) {
            String tz = getPosixTz(userConfig.timeZone.c_str());
            if (!tz.isEmpty()) {
                userConfig.posixTimeZone = tz;
                userConfig.saveToFilesystem();
                needsTimezoneSync = false;
            } else if (userConfig.posixTimeZone.length() == 0) {
                errorTzLookupFailed();
            }
        } else if (userConfig.posixTimeZone.length() == 0) {
            // WiFi didn't connect and we have no idea what timezone we're in
            errorNoWifi();
        }
      }
    }
    
    DEBUG_PRINT("Setting system timezone to %s", userConfig.posixTimeZone.c_str());
    setenv("TZ", userConfig.posixTimeZone.c_str(), 1);
    tzset();

    // Set time

    time_t lastNtpSync = getLastNtpSync();
    if (needsNtpSync || !lastNtpSync) {
        bool syncMandatory = !lastNtpSync || (time_t)difftime(time(nullptr), lastNtpSync) >= ERROR_AFTER_SECONDS_WITHOUT_INTERNET;
        if (startWifi()) {
            if (syncNtp()) {
                time_t updatedTime = getLastNtpSync();
                DEBUG_PRINT_NTP("NTP Sync Time:", updatedTime);
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

    // Sync weather
    if(userConfig.bShowWeather)
    {
      if (needsWeatherSync && startWifi()) {
          OwmResult result = refreshWeather(&userConfig);
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
    }

    stopWifi(); // Power down wifi before updating display to limit current draw from battery

    // Update display if needed
    bool needsDisplayUpdate = now.tm_yday != displayedYDay;
    if(userConfig.bShowWeather)
    {
      needsDisplayUpdate |= lastWeatherSyncTime != getLastWeatherSync() && getSecondsToMidnight(&now) > SECONDS_BEFORE_MIDNIGHT_TO_SYNC_1 * 2;
    }

    if (needsDisplayUpdate) {
        DEBUG_PRINT("Updating display for %d-%d-%d", now.tm_year + 1900, now.tm_mon + 1, now.tm_mday);
        display.update(&now);
        displayedYDay = now.tm_yday;
        if(userConfig.bShowWeather)
        {
          lastWeatherSyncTime = getLastWeatherSync();
        }
    }
    
    // Go to sleep
    time(&t); // Update time measurement
    localtime_r(&t, &now);
    int secondsToMidnight = getSecondsToMidnight(&now) + 1; // +1 to make sure it's actually at or past midnight
    // syncFailed doesn't care about timezone sync because as long as we've got it once it's probably still correct
    bool syncFailed = needsNtpSync;
    
    if(userConfig.bShowWeather)
    {
      syncFailed |= needsWeatherSync;
    }

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
        DEBUG_PRINT_SECONDS("Sleeping until midnight.", secondsToMidnight);
        deepSleep(secondsToMidnight);
    } else if (secondsToMidnight <= SECONDS_BEFORE_MIDNIGHT_TO_SYNC_1 * 2) {
        // Sleep until second NTP sync
        needsNtpSync = true;
        uint64_t secondsToSleep = secondsToMidnight - SECONDS_BEFORE_MIDNIGHT_TO_SYNC_2;
        DEBUG_PRINT_SECONDS("Sleeping for 2nd NTP sync.", secondsToSleep);
        deepSleep(secondsToSleep);
    } else {
        // Sleep until first NTP sync
        needsNtpSync = true;
        if(userConfig.timeZone.length())
        {
          needsTimezoneSync = true;
        }
        if(userConfig.bShowWeather)
        {
          needsWeatherSync = true;
        }
        uint64_t secondsToSleep = secondsToMidnight - SECONDS_BEFORE_MIDNIGHT_TO_SYNC_1;
        DEBUG_PRINT_SECONDS("Sleeping for 1st sync.", secondsToSleep);
        deepSleep(secondsToSleep);
    }
}

void loop()
{
}
