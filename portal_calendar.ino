#include <WiFi.h>
#include <WiFiManager.h>  
#include "global.h"
#include "time.h"
#include "Display.h"
#ifdef SHOW_WEATHER
#include "weather.h"
#endif
#include "qrcodegen/qrcodegen.h"
#include "userConfig.h"

// Config Save & Load (to persist between battery replacements)
UserConfig userConfig;

RTC_DATA_ATTR int bootCount = 0;

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

RTC_DATA_ATTR bool testBool = true;

Display display;
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
    DEBUG_PRINT("Generated QRCodeDataStr");

    // Now that we have the data as a string, we will generate a qr code.
    enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_LOW;  // Error correction level
    DEBUG_PRINT("SetECCLvl");

 
    uint8_t qrcode[qrcodegen_BUFFER_LEN_FOR_VERSION(MAX_QRCODE_VERSION)];
    DEBUG_PRINT("AllocatedBuffer");
    uint8_t tempBuffer[qrcodegen_BUFFER_LEN_FOR_VERSION(MAX_QRCODE_VERSION)];
    DEBUG_PRINT("Allocated Second Buffer");

    bool bOk = qrcodegen_encodeText(QRCodeData.c_str(), tempBuffer, qrcode, errCorLvl,
    qrcodegen_VERSION_MIN, MAX_QRCODE_VERSION, qrcodegen_Mask_AUTO, true);

    DEBUG_PRINT("QRCode Version: %d Length: %d", MAX_QRCODE_VERSION, qrcodegen_BUFFER_LEN_FOR_VERSION(MAX_QRCODE_VERSION));

    
    /*std::vector<bool> qrCodeAsString;
    if(bOk)
    {
      // Finally, we refresh the display to show this information to the user. We could probably write a native QR Code drawing
      // routine for the display... or we could encode the QR Code as text and re-use the existing routine for displaying text. :)
      // (this is definitely not great for heap fragmentation, but also there's like 160kb of it...)

      int border = 4;
      int size = qrcodegen_getSize(qrcode);
      for(int y = 0; y < size; y++)
      {
          String newLine;
          for(int x  = 0; x < size; x++)
          {
            qrCodeAsString += qrcodegen_getModule(qrcode, x, y) ? "#" : "  ";
          }
          qrCodeAsString.push_back(newLine);
      }
    }
    else
    {
      qrCodeAsString.push_back(String("Error generating QRCode."));
    }*/

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
      "automatically connected.",
    };

    // Draw our QR Code + User Prompt to Display
    display.showQRCodeSetup(qrcode, userText);

    userConfig.addParamsToWiFiManager(wifiManager);

    // The startConfigPortal will block until the user confirms information.
    wifiManager.setConfigPortalBlocking(true);
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

void errorNotOnUSBPowerToStartAP()
{
  error({
    "NOT ON USB POWER",
    "",
    "To continue setup the ESP32 needs to be running",
    "on USB power. Please plug in via USB and then",
    "press \"BOOT\" on the back."
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

/**
 * On the EzSBC breakout board, pin 19 is connected to Vusb through an LED and a 1k resistor, meaning it is pulled
 * high if the board is on USB power.
 */
bool isOnUsbPower()
{
    return true;
    pinMode(19, INPUT_PULLDOWN);
    return digitalRead(19) == HIGH;
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
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    #ifdef DEBUG
    Serial.begin(115200);
    // wifiManager.setDebugOutput(true);
    //wifiManager.resetSettings();

    // Convert timestamp and print it over Serial
    time(&t);
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
        // We failed to connect to the WiFi. We're going to ask the user to plug into USB power
        // instead of running the AP without them realizing it.
        if(!isOnUsbPower())
        {
            // If they're not currently on USB power, we will print an error message
            // asking them to plug in via USB, then restart. This function will never
            // return (enters deep sleep).
            errorNotOnUSBPowerToStartAP();
        }

        // If we are on usb power, we're going to start an Access Point that the user
        // can connect to with their phone to configure this display. This will print
        // some info to the screen
        doDeviceConfigurationFlow();

        // If we've made it to here then the user has finished connecting it to a
        // real Access Point, and we can move onto time synchronization.
    }


    // Set timezone

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
