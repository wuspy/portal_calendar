#include "global.h"

#ifndef PORTALCALENDAR_WEATHER_H
#define PORTALCALENDAR_WEATHER_H

/**
 * OWM weather conditions, ordered by severity
 */
enum class WeatherCondition: uint8_t
{
    UNKNOWN = 0,
    CLEAR,
    FEW_CLOUDS,
    SCATTERED_CLOUDS,
    BROKEN_CLOUDS,
    OVERCAST_CLOUDS,
    FOG,
    SCATTERED_SHOWERS,
    SHOWERS,
    THUNDERSTORM,
    FREEZING_RAIN,
    SNOW,
};

enum class OwmResult
{
    SUCCESS,
    INVALID_LOCATION,
    INVALID_API_KEY,
    MALFORMED_RESPONSE,
    NO_RESPONSE,
};

struct OwmLocation
{
    OwmResult result;
    float lat;
    float lon;
    String name;
};

/**
 * Stores aggregated weather data for an entire day
 */
struct DailyWeather {
    WeatherCondition condition;
    int16_t highTemp;
    int16_t lowTemp;
    bool daylight;
    int8_t month;
    int8_t mday;
    int8_t wday;
};

/**
 * Stores a single weather entry from OWM's 5-day/3-hour API
 */
struct WeatherEntry {
    WeatherCondition condition;
    int16_t temp;
    bool daylight;
    int8_t clouds;
    int8_t pop;
    int8_t humidity;
    int8_t month;
    int8_t mday;
    int8_t wday;
    int8_t hour;
    int8_t minute;
};

extern time_t lastWeatherSync;

void getTodaysWeather(int month, int mday, WeatherEntry (&result)[5]);
void get5DayWeather(int month, int mday, int year, DailyWeather (&result)[5]);
OwmResult testApiKey(String apiKey);
OwmLocation queryLocation(String location, String apiKey);
OwmResult refreshWeather();

#endif // PORTALCALENDAR_WEATHER_H
