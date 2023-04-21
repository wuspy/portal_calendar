#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "weather.h"
#include "global.h"
#include "Configuration.h"
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
    DEBUG_PRINT("Failed to find weather entry for %d/%d", month + 1, mday);
    return -1;
}

void getTodaysWeather(int month, int mday, WeatherEntry (&result)[5])
{
    int i = findWeatherEntry(month, mday, Config.getWeatherStartHour());
    int j = 0;
    if (i != -1) {
        for (; j < 5 && i + j < WEATHER_ENTRY_COUNT; ++j) {
            result[j] = weatherEntries[i + j];
        }
    }
    DEBUG_PRINT("Found %d weather entries for %d/%d", j, month + 1, mday);
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

        conditionStart = findWeatherEntry(month, mday, Config.getWeatherStartHour(), j);
        conditionEnd = conditionStart + 4;

        for (; j < WEATHER_ENTRY_COUNT; ++j) {
            entry = &weatherEntries[j];
            if (entry->mday != mday) {
                break;
            }
            // Calculate high/low temp for entire 24-hour day
            day->highTemp = max(day->highTemp, entry->temp);
            day->lowTemp = min(day->lowTemp, entry->temp);
            // Calculate overall condition only for the 12 hour period after the start hour
            if (j >= conditionStart && j <= conditionEnd) {
                ++sampleCount;
                day->condition = max(day->condition, entry->condition);
                clouds = (clouds * (sampleCount - 1) + (float)entry->clouds) / sampleCount;
                daylight = (daylight * (sampleCount - 1) + (entry->daylight ? 100.0 : 0.0)) / sampleCount;
            }
        }        

        DEBUG_PRINT("Found %d weather condition samples for %d/%d/%d", sampleCount, month + 1, mday, year);
        day->daylight = daylight >= 50.0;
        if (day->condition != WeatherCondition::UNKNOWN && day->condition <= WeatherCondition::OVERCAST_CLOUDS) {
            // Use average daily cloud cover for a more representative weather icon
            day->condition = getWeatherConditionByCloudCover((int)clouds);
        }
        advanceDay(month, mday, year);
    }
}

const char* WEATHER_UNIT_NAMES[] = { "imperial", "metric" };

OwmResult refreshWeather()
{
    float latitude = Config.getWeatherLocationLatitude();
    float longitude = Config.getWeatherLocationLongitude();
    char url[200];

    DEBUG_PRINT("Looking up weather for %0.6f,%0.6f from openweathermap", latitude, longitude);
    HTTPClient http;
    unsigned long start = millis();
    http.setConnectTimeout(10000);
    sprintf(
        url,
        "http://api.openweathermap.org/data/2.5/forecast?lat=%0.6f&lon=%0.6f&units=%s&appid=%s",
        latitude,
        longitude,
        urlEncode(WEATHER_UNIT_NAMES[static_cast<size_t>(Config.getWeatherUnits())]),
        urlEncode(Config.getOwmApiKey()).c_str()
    );
    DEBUG_PRINT(url);
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
        DEBUG_PRINT("Request to openweathermap took %lums", millis() - start);
        if (error) {
            DEBUG_PRINT("Failed to parse response: %s", error.c_str());
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
        DEBUG_PRINT("Request to openweathermap failed with %d after %lums", status, millis() - start);
        return status == 401 ? OwmResult::INVALID_API_KEY : OwmResult::NO_RESPONSE;
    }
}

OwmResult testApiKey(String apiKey)
{
    char url[200];
    sprintf(url, "http://api.openweathermap.org/data/2.5/weather?lat=51.48&lon=0&appid=%s", urlEncode(apiKey).c_str());

    HTTPClient http;
    http.begin(url);
    switch (http.GET()) {
        case 200:
            return OwmResult::SUCCESS;
        case 401:
            return OwmResult::INVALID_API_KEY;
        default:
            return OwmResult::NO_RESPONSE;
    }
}

OwmLocation queryLocation(String location, String apiKey)
{
    char buffer[200];
    // Use OWM's geocoding API to lookup the coordinates for the provided location
    DEBUG_PRINT("Looking up lat,long for '%s' from openweathermap", location.c_str());
    HTTPClient http;
    unsigned long start = millis();
    http.setConnectTimeout(10000);
    sprintf(
        buffer,
        "http://api.openweathermap.org/geo/1.0/direct?q=%s&limit=1&appid=%s",
        urlEncode(location).c_str(),
        urlEncode(apiKey).c_str()
    );
    DEBUG_PRINT(buffer);
    http.begin(buffer);
    int status = http.GET();
    if (status == 200) {
        StaticJsonDocument<128> filter;
        filter[0]["name"] = true;
        filter[0]["state"] = true;
        filter[0]["country"] = true;
        filter[0]["lat"] = true;
        filter[0]["lon"] = true;
        DynamicJsonDocument response(1000);
        DeserializationError error = deserializeJson(response, http.getStream(), DeserializationOption::Filter(filter));
        http.end();
        DEBUG_PRINT("Request to openweathermap took %lums", millis() - start);
        if (error) {
            DEBUG_PRINT("Failed to parse response: %s", error.c_str());
            return {
                .result=OwmResult::MALFORMED_RESPONSE,
                .lat=0,
                .lon=0,
                .name="",
            };
        }
        JsonVariant result = response[0];
        if (result.isNull()) {
            DEBUG_PRINT("No location results from openweathermap");
            return {
                .result=OwmResult::INVALID_LOCATION,
                .lat=0,
                .lon=0,
                .name="",
            };
        }

        const char *name = result["name"].as<const char*>();
        const char *country = result["country"].as<const char*>();
        float latitude = result["lat"].as<float>();
        float longitude = result["lon"].as<float>();

        if (result["state"].isNull()) {
            sprintf(buffer, "%s, %s", name, country);
        } else {
            const char *state = result["state"].as<const char*>();
            sprintf(buffer, "%s, %s, %s", name, state, country);
        }

        DEBUG_PRINT("Found location %s @ %0.6f,%0.6f", buffer, latitude, longitude);

        return {
            .result=OwmResult::SUCCESS,
            .lat=latitude,
            .lon=longitude,
            .name=buffer,
        };
    } else {
        http.end();
        DEBUG_PRINT("Request to openweathermap failed with %d after %lums", status, millis() - start);
        return {
            .result=status == 401 ? OwmResult::INVALID_API_KEY : OwmResult::NO_RESPONSE,
            .lat=0,
            .lon=0,
            .name="",
        };
    }
}
