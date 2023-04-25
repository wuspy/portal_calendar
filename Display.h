#include "Arduino.h"
#include "DisplayGDEW075T7.h"
#include "resources/image.h"
#include "global.h"
#include "weather.h"

#ifndef PORTALCALENDAR_DISPLAY_H
#define PORTALCALENDAR_DISPLAY_H

class Display
{
public:
    Display();
    ~Display();
    void error(std::initializer_list<String> messageLines, bool willRetry);
    void update(const tm *now, bool showWeather);
    void testChamberIcons();

private:
    void init();
    const Image* getWeatherConditionIcon(WeatherCondition condition, bool day);
    void drawWeatherInfoText(const char* text, const Image* symbol, int32_t x, int32_t y);
    void drawDailyWeather(const DailyWeather& weather, int32_t x);
    void drawWeatherEntry(const WeatherEntry& weather, int32_t x);
    void drawChamberIcon(const Image& icon, int32_t x, int32_t y);
    DisplayGDEW075T7 *_display;
};

#endif // PORTALCALENDAR_DISPLAY_H
