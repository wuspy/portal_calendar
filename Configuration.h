#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <WiFi.h>
#include "global.h"

#ifndef PORTALCALENDAR_CONFIGURATION_H
#define PORTALCALENDAR_CONFIGURATION_H

class ConfigurationClass
{
public:
    ~ConfigurationClass();
    void begin();
    void reset();
    void startConfigServer();
    bool isConfigured();

    String getWifiSsid();
    String getWifiPass();
    String getHostname();
    bool getShowDay();
    bool getShowMonth();
    bool getShowYear();
    String getTimezoneName();
    String getLocale();
    String getPrimaryNtpServer();
    String getSecondaryNtpServer();
    String getPrimaryTimezonedServer();
    String getSecondaryTimezonedServer();
    bool getWeatherEnabled();
    String getOwmApiKey();
    String getWeatherLocationName();
    float getWeatherLocationLatitude();
    float getWeatherLocationLongitude();
    WeatherDisplayType getWeatherDisplayType();
    WeatherUnits getWeatherUnits();
    WeatherSecondaryInfo getWeatherSecondaryInfo();
    uint8_t getWeatherStartHour();
    bool getShow24HourTime();
    float getMaxRtcCorrectionFactor();
    bool getTwoNtpSyncsPerDay();

    inline bool wasSaved()
    {
        return _wasSaved;
    }

    inline bool isOnUsbPower()
    {
        return digitalRead(PD_PIN) == PD_PIN_STATE;
    }

private:
    QueueHandle_t _deferredRequestQueue;
    Preferences _prefs;
    DNSServer *_dnsServer = nullptr;
    AsyncWebServer *_httpServer = nullptr;
    bool _shutdown = true;
    bool _wifiConnecting = false;
    bool _wasSaved = false;

    void deferRequest(AsyncWebServerRequest *request, std::function<void(void)> handler);
    void connectToSavedWifi();
    bool isApRequest(AsyncWebServerRequest *request);

    template<typename T> T prefs_getEnum(const char* key, T defaultValue);
    void prefs_putJsonBool(const JsonObject& json, const char* key);
    void prefs_putJsonString(const JsonObject& json, const char* key, unsigned int minLength = 0, unsigned int maxLength = 1024);
    void prefs_putJsonFloat(const JsonObject& json, const char* key, float min, float max);
    void prefs_putJsonUChar(const JsonObject& json, const char* key, uint8_t min, uint8_t max);
    template<typename T> void prefs_putJsonEnum(const JsonObject& json, const char* key, std::initializer_list<T> values);

    AsyncCallbackWebHandler& on(const char* uri, WebRequestMethodComposite method, ArRequestHandlerFunction onRequest);
    AsyncCallbackJsonWebHandler& on(const char* uri, WebRequestMethodComposite method, size_t maxJsonBufferSize, ArJsonRequestHandlerFunction onRequest);
};

extern ConfigurationClass Config;

#endif // PORTALCALENDAR_CONFIGURATION_H
