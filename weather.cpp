#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "weather.h"
#include "global.h"
#include "time_util.h"

const WeatherEntry EMPTY_WEATHER_ENTRY = {
    .condition = WeatherCondition::UNKNOWN,
    .temp = INT16_MAX,
    .daylight = false,
    .clouds = -1,
    .pop = -1,
    .humidity = -1,
    .month = -1,
    .mday = -1,
    .wday = -1,
    .hour = -1,
    .minute = -1,
};

const DailyWeather EMPTY_DAILY_WEATHER = {
    .condition = WeatherCondition::UNKNOWN,
    .highTemp = INT16_MIN,
    .lowTemp = INT16_MAX,
    .daylight = false,
    .month = -1,
    .mday = -1,
    .wday = -1,
};

RTC_DATA_ATTR time_t lastWeatherSync = 0;
RTC_DATA_ATTR float latitude = WEATHER_LOCATION_LATITUDE;
RTC_DATA_ATTR float longitude = WEATHER_LOCATION_LONGITUDE;
RTC_DATA_ATTR time_t sunriseTime = 0;
RTC_DATA_ATTR time_t sunsetTime = 0;
#define WEATHER_ENTRY_COUNT 40
RTC_DATA_ATTR WeatherEntry weatherEntries[WEATHER_ENTRY_COUNT] = {
    EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY,
    EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY,
    EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY,
    EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY,
    EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY,
    EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY,
    EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY,
    EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY, EMPTY_WEATHER_ENTRY,
};

String urlEncode(String str)
{
    const char* hex = "0123456789ABCDEF";
    String result;
    result.reserve(str.length());
    for (char c : str) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            result += c;
        } else if (c == ' ') {
            result += '+';
        } else {
            result += '%';
            result += hex[c >> 4];
            result += hex[c & 0xf];
        }
    }
    return result;
}

bool isDaylight(time_t t)
{
    // Adjust t to be on the same day as the current sunrise/sunset time
    if (sunriseTime > t) {
        t += ((sunriseTime - t) / SECONDS_PER_DAY + 1) * SECONDS_PER_DAY;
    } else if (sunsetTime < t) {
        t -= ((t - sunsetTime) / SECONDS_PER_DAY + 1) * SECONDS_PER_DAY;
    }
    return t >= sunriseTime && t < sunsetTime;
}

WeatherCondition getWeatherConditionByCloudCover(int clouds)
{
    if (clouds < 10) {
        return WeatherCondition::CLEAR;
    } else if (clouds < 26) {
        return WeatherCondition::FEW_CLOUDS;
    } else if (clouds < 51) {
        return WeatherCondition::SCATTERED_CLOUDS;
    } else if (clouds < 85) {
        return WeatherCondition::BROKEN_CLOUDS;
    } else {
        return WeatherCondition::OVERCAST_CLOUDS;
    }
}

WeatherCondition parseOWMWeatherConditionId(int id)
{
    if (id < 200) {
        return WeatherCondition::UNKNOWN;
    } else if (id < 300) {
        return WeatherCondition::THUNDERSTORM;
    } else if (id < 400) {
        return WeatherCondition::SHOWERS;
    } else if (id < 505) {
        return WeatherCondition::SCATTERED_SHOWERS;
    } else if (id == 511) {
        return WeatherCondition::FREEZING_RAIN;
    } else if (id < 600) {
        return WeatherCondition::SHOWERS;
    } else if (id < 700) {
        return WeatherCondition::SNOW;
    } else if (id < 800) {
        return WeatherCondition::FOG;
    } else if (id == 800) {
        return WeatherCondition::CLEAR;
    } else if (id == 801) {
        return WeatherCondition::FEW_CLOUDS;
    } else if (id == 802) {
        return WeatherCondition::SCATTERED_CLOUDS;
    } else if (id == 803) {
        return WeatherCondition::BROKEN_CLOUDS;
    } else if (id == 804) {
        return WeatherCondition::OVERCAST_CLOUDS;
    } else {
        return WeatherCondition::UNKNOWN;
    }
}

void parseOWMWeatherEntry(const JsonVariant& data, WeatherEntry& entry)
{
    time_t time = data["dt"].as<time_t>();
    tm localtime;
    localtime_r(&time, &localtime);

    entry.condition = parseOWMWeatherConditionId(data["weather"][0]["id"].as<int>());
    entry.temp = (int16_t)round(data["main"]["temp"].as<float>());
    entry.daylight = isDaylight(time);
    entry.clouds = data["clouds"]["all"].as<uint8_t>();
    entry.pop = (uint8_t)round(data["pop"].as<float>() * 100.0);
    entry.humidity = data["main"]["humidity"].as<uint8_t>();
    entry.month = localtime.tm_mon;
    entry.mday = localtime.tm_mday;
    entry.wday = localtime.tm_wday;
    entry.hour = localtime.tm_hour;
    entry.minute = localtime.tm_min;
}

/**
 * Gets the index of the weather entry closest to the specified hour on the specified day.
 * Returns -1 if no entries exist for the specified day.
 */
int findWeatherEntry(int month, int mday, int hour, int startIndex = 0)
{
    const int minuteOfDay = hour * 60;
    WeatherEntry *current;
    bool foundDay = false;
    for (int i = max(startIndex, 0); i < WEATHER_ENTRY_COUNT; ++i) {
        current = &weatherEntries[i];
        if (current->month == month && current->mday == mday) {
            foundDay = true;
            // Reach the closest entry to the target hour. Since OWM returns data in 3 hour intervals, the closest
            // entry will be at most 90 minutes off. However this also needs to consider the closest entry being far
            // ahead of the target hour, since OWM doesn't return past data and it could have been fetched at any time.
            if (current->hour >= hour || abs((int)current->hour * 60 + (int)current->minute - minuteOfDay) <= 90) {
                return i;
            }
        } else if (foundDay) {
            // Hour is greater than any hour returned by OWM for this day, so return the last entry
            return i - 1;
        }
    }
    log_w("Failed to find weather entry for %d/%d", month + 1, mday);
    return -1;
}

void getTodaysWeather(int month, int mday, WeatherEntry (&result)[5])
{
    int i = findWeatherEntry(month, mday, WEATHER_START_HOUR);
    int j = 0;
    if (i != -1) {
        for (; j < 5 && i + j < WEATHER_ENTRY_COUNT; ++j) {
            result[j] = weatherEntries[i + j];
        }
    }
    log_i("Found %d weather entries for %d/%d", j, month + 1, mday);
    for (; j < 5; ++j) {
        result[j] = EMPTY_WEATHER_ENTRY;
    }
}

void get5DayWeather(int month, int mday, int year, DailyWeather (&result)[5])
{
    DailyWeather *day;
    WeatherEntry *entry;
    int j = 0;
    int conditionStart, conditionEnd;
    float clouds, daylight;
    int sampleCount;

    for (int i = 0; i < 5; ++i) {
        day = &result[i];
        *day = EMPTY_DAILY_WEATHER;

        j = findWeatherEntry(month, mday, 0, j);
        if (j == -1) {
            j = 0;
            advanceDay(month, mday, year);
            continue;
        }

        day->month = month;
        day->mday = mday;
        day->wday = weatherEntries[j].wday;

        clouds = 0.0;
        daylight = 0.0;
        sampleCount = 0;

        conditionStart = findWeatherEntry(month, mday, WEATHER_START_HOUR, j);
        conditionEnd = conditionStart + 4;

        for (; j < WEATHER_ENTRY_COUNT; ++j) {
            entry = &weatherEntries[j];
            if (entry->mday != mday) {
                break;
            }
            // Calculate high/low temp for entire 24-hour day
            day->highTemp = max(day->highTemp, entry->temp);
            day->lowTemp = min(day->lowTemp, entry->temp);
            // Calculate overall condition only for the 12 hour period after WEATHER_START_HOUR
            if (j >= conditionStart && j <= conditionEnd) {
                ++sampleCount;
                day->condition = max(day->condition, entry->condition);
                clouds = (clouds * (sampleCount - 1) + (float)entry->clouds) / sampleCount;
                daylight = (daylight * (sampleCount - 1) + (entry->daylight ? 100.0 : 0.0)) / sampleCount;
            }
        }        

        log_i("Found %d weather condition samples for %d/%d/%d", sampleCount, month + 1, mday, year);
        day->daylight = daylight >= 50.0;
        if (day->condition != WeatherCondition::UNKNOWN && day->condition <= WeatherCondition::OVERCAST_CLOUDS) {
            // Use average daily cloud cover for a more representative weather icon
            day->condition = getWeatherConditionByCloudCover((int)clouds);
        }
        advanceDay(month, mday, year);
    }
}

OwmResult refreshWeather()
{
    char url[200];
    if (latitude == 0.0 || longitude == 0.0) {
        // Use OWM's geocoding API to lookup the coordinates for the provided location
        log_i("Looking up lat,long for '" WEATHER_LOCATION "' from openweathermap");
        HTTPClient http;
        unsigned long start = millis();
        http.setConnectTimeout(10000);
        sprintf(
            url,
            "http://api.openweathermap.org/geo/1.0/direct?q=%s&limit=1&appid=%s",
            urlEncode(WEATHER_LOCATION).c_str(),
            urlEncode(OPENWEATHERMAP_API_KEY).c_str()
        );
        http.begin(url);
        int status = http.GET();
        if (status == 200) {
            DynamicJsonDocument response(1000);
            DeserializationError error = deserializeJson(response, http.getString());
            http.end();
            log_i("Request to openweathermap took %lums", millis() - start);
            if (error) {
                log_e("Failed to parse response: %s", error.c_str());
                return OwmResult::MALFORMED_RESPONSE;
            }
            JsonVariant result = response[0];
            if (result.isNull()) {
                log_w("No location results from openweathermap");
                return OwmResult::INVALID_LOCATION;
            }
            latitude = result["lat"].as<float>();
            longitude = result["lon"].as<float>();
            log_i("Found location %s, %s @ %0.6f,%0.6f", result["name"].as<const char*>(), result["country"].as<const char*>(), latitude, longitude);
        } else {
            http.end();
            log_e("Request to openweathermap failed with %d after %lums", status, millis() - start);
            return status == 401 ? OwmResult::INVALID_API_KEY : OwmResult::NO_RESPONSE;
        }
    }

    log_i("Looking up weather for %0.6f,%0.6f from openweathermap", latitude, longitude);
    HTTPClient http;
    unsigned long start = millis();
    http.setConnectTimeout(10000);
    sprintf(
        url,
        "http://api.openweathermap.org/data/2.5/forecast?lat=%0.6f&lon=%0.6f&units=%s&appid=%s",
        latitude,
        longitude,
        urlEncode(WEATHER_UNITS).c_str(),
        urlEncode(OPENWEATHERMAP_API_KEY).c_str()
    );
    http.begin(url);
    int status = http.GET();
    if (status == 200) {
        StaticJsonDocument<256> filter;
        filter["city"]["sunrise"] = true;
        filter["city"]["sunset"] = true;
        filter["cnt"] = true;
        filter["list"][0]["dt"] = true;
        filter["list"][0]["main"]["temp"] = true;
        filter["list"][0]["main"]["humidity"] = true;
        filter["list"][0]["clouds"]["all"] = true;
        filter["list"][0]["weather"][0]["id"] = true;
        filter["list"][0]["pop"] = true;

        DynamicJsonDocument response(10000);
        DeserializationError error = deserializeJson(response, http.getStream(), DeserializationOption::Filter(filter));
        http.end();
        log_i("Request to openweathermap took %lums", millis() - start);
        if (error) {
            log_e("Failed to parse response: %s", error.c_str());
            return OwmResult::MALFORMED_RESPONSE;
        }
        
        // Get sunrise and sunet hours for this location
        JsonVariant city = response["city"];
        sunriseTime = city["sunrise"].as<time_t>();
        sunsetTime = city["sunset"].as<time_t>();

        const int cnt = response["cnt"].as<int>();
        JsonVariant list = response["list"];
        int i = 0;
        for (; i < min(cnt, WEATHER_ENTRY_COUNT); ++i) {
            parseOWMWeatherEntry(list[i], weatherEntries[i]);
        }
        // In case owm returned less results than expected, clear the rest
        for (; i < WEATHER_ENTRY_COUNT; ++i) {
            weatherEntries[i] = EMPTY_WEATHER_ENTRY;
        }
        time(&lastWeatherSync);
        return OwmResult::SUCCESS;
    } else {
        http.end();
        log_e("Request to openweathermap failed with %d after %lums", status, millis() - start);
        return status == 401 ? OwmResult::INVALID_API_KEY : OwmResult::NO_RESPONSE;
    }
}
