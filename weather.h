#include "global.h"

#ifndef PORTALCALENDAR_WEATHER_H
#define PORTALCALENDAR_WEATHER_H

#if !defined(WEATHER_LOCATION) && (!defined(WEATHER_LOCATION_LATITUDE) || !defined(WEATHER_LOCATION_LONGITUDE))
#error No weather location configured
#endif

#ifndef WEATHER_START_HOUR
#error WEATHER_START_HOUR must be defined
#endif

#if (WEATHER_START_HOUR < 0 || WEATHER_START_HOUR > 23)
#error Weather start hour must be between 0 and 23
#endif

#ifndef OPENWEATHERMAP_API_KEY
#error OPENWEATHERMAP_API_KEY must be set in order to get weather information
#endif

#ifndef WEATHER_LOCATION_LATITUDE
#define WEATHER_LOCATION_LATITUDE 0.0
#endif

#ifndef WEATHER_LOCATION_LONGITUDE
#define WEATHER_LOCATION_LONGITUDE 0.0
#endif

/**
 * OWM weather conditions, ordered by severity
 */
enum WeatherCondition: uint8_t
{
    WEATHER_CONDITION_UNKNOWN = 0,
    WEATHER_CONDITION_CLEAR,
    WEATHER_CONDITION_FEW_CLOUDS,
    WEATHER_CONDITION_SCATTERED_CLOUDS,
    WEATHER_CONDITION_BROKEN_CLOUDS,
    WEATHER_CONDITION_OVERCAST_CLOUDS,
    WEATHER_CONDITION_FOG,
    WEATHER_CONDITION_SCATTERED_SHOWERS,
    WEATHER_CONDITION_SHOWERS,
    WEATHER_CONDITION_THUNDERSTORM,
    WEATHER_CONDITION_FREEZING_RAIN,
    WEATHER_CONDITION_SNOW,
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

time_t getLastWeatherSync();
void getTodaysWeather(int month, int mday, WeatherEntry (&result)[5]);
void get5DayWeather(int month, int mday, int year, DailyWeather (&result)[5]);
bool refreshWeather();

#endif // PORTALCALENDAR_WEATHER_H
