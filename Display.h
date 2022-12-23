#include "Arduino.h"
#include "DisplayGDEW075T7.h"
#include "resource/image.h"
#include "global.h"
#ifdef SHOW_WEATHER
#include "weather.h"
#endif

#ifndef PORTALCALENDAR_DISPLAY_H
#define PORTALCALENDAR_DISPLAY_H

class Display
{
public:
    Display();
    ~Display();
    void error(std::initializer_list<String> messageLines, bool willRetry);
    void update(const tm *now);
    #ifndef SHOW_WEATHER
    void testChamberIcons();
    #endif // SHOW_WEATHER

private:
    void init();
    #ifdef SHOW_WEATHER
    const Image* getWeatherConditionIcon(WeatherCondition condition, bool day);
    void drawWeatherInfoText(const char* text, const Image* symbol, int32_t x, int32_t y);
    void drawDailyWeather(const DailyWeather& weather, int32_t x);
    void drawWeatherEntry(const WeatherEntry& weather, int32_t x);
    #else
    void drawChamberIcon(const Image& icon, int32_t x, int32_t y);
    #endif // SHOW_WEATHER
    DisplayGDEW075T7 *_display;
};

#endif // PORTALCALENDAR_DISPLAY_H
