#include "Arduino.h"
#include "DisplayGDEW075T7.h"
#include "resources/image.h"
#include "global.h"
#include "weather.h"
#include "localization.h"

#ifndef PORTALCALENDAR_DISPLAY_H
#define PORTALCALENDAR_DISPLAY_H

class DisplayClass
{
public:
    ~DisplayClass();
    void error(std::initializer_list<String> messageLines, bool willRetry);
    void update(const tm *now, const Locale& locale, bool showWeather);
    void showConfigInstructions();
    void showConfigServerScreen(String ssid, String password, String hostname);
    #ifdef DEV_WEBSERVER
    void showDevWebserverScreen(String ssid, IPAddress localIp);
    #endif
    #ifdef DEBUG
    void testChamberIcons();
    #endif

private:
    void init();
    const Image* getWeatherConditionIcon(WeatherCondition condition, bool day);
    void drawWeatherInfoText(const char* text, const Image* symbol, int32_t x, int32_t y);
    void drawDailyWeather(const DailyWeather& weather, int32_t x, const Locale& locale);
    void drawWeatherEntry(const WeatherEntry& weather, int32_t x);
    void drawChamberIcon(const Image& icon, int32_t x, int32_t y);
    void drawStandardSeparators();
    void drawChamberNumber(int number, int total);
    void drawApertureLogo();
    DisplayGDEW075T7 *_display = nullptr;
};

extern DisplayClass Display;

#endif // PORTALCALENDAR_DISPLAY_H
