#include "Display.h"
#include "global.h"
#include "time.h"

#include "resource/font/regular.h"
#include "resource/font/chamber_number.h"

#include "resource/aperture_logo.h"
#include "resource/progress_bar.h"
#include "resource/error.h"

#ifdef SHOW_WEATHER
#include "resource/font/weather_frame.h"
#include "resource/font/weather_info.h"
#include "resource/weather_frame.h"
#include "resource/weather_frame_empty.h"
#include "resource/weather_cloudy.h"
#include "resource/weather_fog.h"
#include "resource/weather_thunderstorms.h"
#include "resource/weather_showers.h"
#include "resource/weather_snow.h"
#include "resource/weather_day_clear.h"
#include "resource/weather_night_clear.h"
#include "resource/weather_partly_cloudy_day.h"
#include "resource/weather_partly_cloudy_night.h"
#include "resource/weather_scattered_showers_day.h"
#include "resource/weather_scattered_showers_night.h"
#else
#include "resource/cube_dispenser_on.h"
#include "resource/cube_dispenser_off.h"
#include "resource/cube_hazard_on.h"
#include "resource/cube_hazard_off.h"
#include "resource/pellet_hazard_on.h"
#include "resource/pellet_hazard_off.h"
#include "resource/pellet_catcher_on.h"
#include "resource/pellet_catcher_off.h"
#include "resource/water_hazard_on.h"
#include "resource/water_hazard_off.h"
#include "resource/fling_enter_on.h"
#include "resource/fling_enter_off.h"
#include "resource/fling_exit_on.h"
#include "resource/fling_exit_off.h"
#include "resource/turret_hazard_on.h"
#include "resource/turret_hazard_off.h"
#include "resource/dirty_water_on.h"
#include "resource/dirty_water_off.h"
#include "resource/cake_on.h"
#include "resource/cake_off.h"
#endif // SHOW_WEATHER

#define ICON_SIZE 64
#define ICON_SPACING 9
#define LEFT 82
#define RIGHT (LEFT + ICON_SIZE * 5 + ICON_SPACING * 4)
#define WIDTH (RIGHT - LEFT)
#define ICON_TOP 550
// Part of the physical screen is covered by the bezel to change the aspect ratio from 15:9 to 16:9,
// this marks the horizontal centerline of the visible area
#define H_CENTER 225

const char* MONTHS[] = {
    "JANUARY",
    "FEBRUARY",
    "MARCH",
    "APRIL",
    "MAY",
    "JUNE",
    "JULY",
    "AUGUST",
    "SEPTEMBER",
    "OCTOBER",
    "NOVEMBER",
    "DECEMBER",
};

const char* DAYS[] = {
    "SUNDAY",
    "MONDAY",
    "TUESDAY",
    "WEDNESDAY",
    "THURSDAY",
    "FRIDAY",
    "SATURDAY",
};

const char* DAYS_ABBREVIATIONS[] = {
    "SUN",
    "MON",
    "TUE",
    "WED",
    "THU",
    "FRI",
    "SAT",
};

Display::Display()
{
    _display = nullptr;
}

Display::~Display()
{
    if (_display) {
        delete _display;
    }
}

void Display::init()
{
    if (!_display) {
        _display = new DisplayGDEW075T7(SPI_BUS, CS_PIN, RESET_PIN, DC_PIN, BUSY_PIN);
        _display->setRotation(DisplayGDEW075T7::ROTATION_270);
        _display->setAlpha(DisplayGDEW075T7::WHITE);
    } else {
        _display->clear();
    }
}

void Display::update(const tm *now)
{
    init();
    char buffer[10];
    const int year = now->tm_year + 1900;
    const int daysInMonth = getDaysInMonth(now->tm_mon, year);

    // Static lines
    _display->drawHLine(LEFT, 50, WIDTH, 2, DisplayGDEW075T7::BLACK, DisplayGDEW075T7::TOP_LEFT);
    _display->drawHLine(LEFT, 430, WIDTH, 2, DisplayGDEW075T7::BLACK, DisplayGDEW075T7::TOP_LEFT);
    _display->drawHLine(LEFT, 538, WIDTH, 2, DisplayGDEW075T7::BLACK, DisplayGDEW075T7::TOP_LEFT);

    // Aperture logo
    _display->drawImage(IMG_APERTURE_LOGO, LEFT, 740);

    // BIG date
    sprintf(buffer, "%02d", now->tm_mday);
    _display->drawText(buffer, FONT_CHAMBER_NUMBER, LEFT, 16, 10);

    // Small "XX/XX" date
    sprintf(buffer, "%02d/%02d", now->tm_mday, daysInMonth);
    _display->drawText(buffer, FONT_REGULAR, LEFT, 394);

    #ifdef SHOW_DAY
    // Day name
    _display->drawText(DAYS[now->tm_wday], FONT_REGULAR, RIGHT, 394, 0, DisplayGDEW075T7::TOP_RIGHT);
    #endif

    #ifdef SHOW_MONTH
    // Month name
    _display->drawText(MONTHS[now->tm_mon], FONT_REGULAR, LEFT, 14, 0);
    #endif

    #ifdef SHOW_YEAR
    // Year
    sprintf(buffer, "%d", year);
    _display->drawText(buffer, FONT_REGULAR, RIGHT, 14, 0, DisplayGDEW075T7::TOP_RIGHT);
    #endif

    // Progress bar
    _display->drawImage(IMG_PROGRESS_BAR, LEFT, 438);
    int32_t progressWidth = IMG_PROGRESS_BAR.width * now->tm_mday / daysInMonth;
    _display->fillRect(LEFT + progressWidth, 438, IMG_PROGRESS_BAR.width - progressWidth, IMG_PROGRESS_BAR.height, DisplayGDEW075T7::WHITE);

    #ifdef SHOW_WEATHER

    #if WEATHER_DISPLAY_TYPE == 1

    DailyWeather weather[5];
    get5DayWeather(now->tm_mon, now->tm_mday, year, weather);

    for (int i = 0; i < 5; ++i) {
        drawDailyWeather(weather[i], i);
    }

    #elif WEATHER_DISPLAY_TYPE == 2

    WeatherEntry weather[5];
    getTodaysWeather(now->tm_mon, now->tm_mday, weather);

    for (int i = 0; i < 5; ++i) {
        drawWeatherEntry(weather[i], i);
    }

    #endif

    #else

    // Icons, random for now
    srand(millis());
    drawIcon(rand() % 2 ? IMG_CUBE_DISPENSER_ON : IMG_CUBE_DISPENSER_OFF, 0, 0);
    drawIcon(rand() % 2 ? IMG_CUBE_HAZARD_ON : IMG_CUBE_HAZARD_OFF, 1, 0);
    drawIcon(rand() % 2 ? IMG_PELLET_HAZARD_ON : IMG_PELLET_HAZARD_OFF, 2, 0);
    drawIcon(rand() % 2 ? IMG_PELLET_CATCHER_ON : IMG_PELLET_CATCHER_OFF, 3, 0);
    drawIcon(rand() % 2 ? IMG_WATER_HAZARD_ON : IMG_WATER_HAZARD_OFF, 4, 0);
    drawIcon(rand() % 2 ? IMG_FLING_ENTER_ON : IMG_FLING_ENTER_OFF, 0, 1);
    drawIcon(rand() % 2 ? IMG_FLING_EXIT_ON : IMG_FLING_EXIT_OFF, 1, 1);
    drawIcon(rand() % 2 ? IMG_TURRET_HAZARD_ON : IMG_TURRET_HAZARD_OFF, 2, 1);
    drawIcon(rand() % 2 ? IMG_DIRTY_WATER_ON : IMG_DIRTY_WATER_OFF, 3, 1);
    drawIcon(rand() % 2 ? IMG_CAKE_ON : IMG_CAKE_OFF, 4, 1);

    #endif
    
    _display->refresh();
}

#ifdef SHOW_WEATHER

const Image* Display::getWeatherConditionIcon(WeatherCondition condition, bool day)
{
    switch (condition) {
        case WEATHER_CONDITION_CLEAR:
        case WEATHER_CONDITION_FEW_CLOUDS:
            return day ? &IMG_WEATHER_DAY_CLEAR : &IMG_WEATHER_NIGHT_CLEAR;
        case WEATHER_CONDITION_SCATTERED_CLOUDS:
        case WEATHER_CONDITION_BROKEN_CLOUDS:
            return day ? &IMG_WEATHER_PARTLY_CLOUDY_DAY : &IMG_WEATHER_PARTLY_CLOUDY_NIGHT;
        case WEATHER_CONDITION_OVERCAST_CLOUDS:
            return &IMG_WEATHER_CLOUDY;
        case WEATHER_CONDITION_SCATTERED_SHOWERS:
            return day ? &IMG_WEATHER_SCATTERED_SHOWERS_DAY : &IMG_WEATHER_SCATTERED_SHOWERS_NIGHT;
        case WEATHER_CONDITION_SHOWERS:
            return &IMG_WEATHER_SHOWERS;
        case WEATHER_CONDITION_THUNDERSTORM:
            return &IMG_WEATHER_THUNDERSTORMS;
        case WEATHER_CONDITION_FOG:
            return &IMG_WEATHER_FOG;
        case WEATHER_CONDITION_FREEZING_RAIN:
        case WEATHER_CONDITION_SNOW:
            return &IMG_WEATHER_SNOW;
        default:
            return nullptr;
    }
}

void Display::drawDailyWeather(const DailyWeather& weather, int32_t x)
{
    x = LEFT + x * (ICON_SIZE + ICON_SPACING);

    if (weather.mday == -1) {
        _display->drawImage(IMG_WEATHER_FRAME_EMPTY, x, ICON_TOP);
        return;
    }

    char text[10];

    // Draw frame
    _display->drawImage(IMG_WEATHER_FRAME, x, ICON_TOP);

    // Draw condition icon
    const Image* icon = getWeatherConditionIcon(weather.condition, weather.daylight);
    if (icon) {
        _display->drawImage(*icon, x + 2, ICON_TOP + 21);
    }

    // Draw day
    _display->setAlpha(DisplayGDEW075T7::BLACK);
    _display->drawText(DAYS_ABBREVIATIONS[weather.wday], FONT_WEATHER_FRAME, x + 5, ICON_TOP, 0);
    sprintf(text, "%d", weather.mday);
    _display->drawText(text, FONT_WEATHER_FRAME, x + 64 - 5, ICON_TOP, 0, DisplayGDEW075T7::TOP_RIGHT);
    _display->setAlpha(DisplayGDEW075T7::WHITE);

    // Draw high temp
    sprintf(text, "%d", weather.highTemp);
    _display->drawText(text, FONT_WEATHER_INFO, x + 32, ICON_TOP + 83, 0, DisplayGDEW075T7::TOP_CENTER);

    // Draw low temp
    sprintf(text, "%d", weather.lowTemp);
    _display->drawText(text, FONT_WEATHER_INFO, x + 32, ICON_TOP + 108, 0, DisplayGDEW075T7::TOP_CENTER);
}

void Display::drawWeatherEntry(const WeatherEntry& weather, int32_t x)
{
    x = LEFT + x * (ICON_SIZE + ICON_SPACING);

    if (weather.mday == -1) {
        _display->drawImage(IMG_WEATHER_FRAME_EMPTY, x, ICON_TOP);
        return;
    }

    char text[10];

    // Draw frame
    _display->drawImage(IMG_WEATHER_FRAME, x, ICON_TOP);

    // Draw condition icon
    const Image* icon = getWeatherConditionIcon(weather.condition, weather.daylight);
    if (icon) {
        _display->drawImage(*icon, x + 2, ICON_TOP + 21);
    }

    // Draw time
    #ifdef SHOW_24_HOUR_TIME
    sprintf(text, "%02d:%02d", weather.hour, weather.minute);
    #else
    int8_t hour12 = weather.hour % 12;
    if (hour12 == 0) {
        hour12 = 12;
    }
    sprintf(text, "%d:%02d %s", hour12, weather.minute, weather.hour > 11 ? "PM" : "AM");
    #endif

    _display->setAlpha(DisplayGDEW075T7::BLACK);
    _display->drawText(text, FONT_WEATHER_FRAME, x + 32, ICON_TOP, 0, DisplayGDEW075T7::TOP_CENTER);
    _display->setAlpha(DisplayGDEW075T7::WHITE);

    // Draw temperature
    sprintf(text, "%d", weather.temp);
    _display->drawText(text, FONT_WEATHER_INFO, x + 32, ICON_TOP + 83, 0, DisplayGDEW075T7::TOP_CENTER);

    // Draw secondary info
    #if SECONDARY_WEATHER_INFORMATION == 1 // Precipitation
    sprintf(text, "%d", weather.pop);
    _display->drawText(text, FONT_WEATHER_INFO, x + 32, ICON_TOP + 108, 0, DisplayGDEW075T7::TOP_CENTER);
    #elif SECONDARY_WEATHER_INFORMATION == 2 // Humidity
    sprintf(text, "%d", weather.humidity);
    _display->drawText(text, FONT_WEATHER_INFO, x + 32, ICON_TOP + 108, 0, DisplayGDEW075T7::TOP_CENTER);
    #endif
}

#endif // SHOW_WEATHER

void Display::drawIcon(const Image& icon, int32_t x, int32_t y)
{
    _display->drawImage(
        icon,
        LEFT + x * (ICON_SIZE + ICON_SPACING),
        ICON_TOP + y * (ICON_SIZE + ICON_SPACING)
    );
}

void Display::error(String message)
{
    init();
    _display->drawImage(IMG_ERROR, H_CENTER, _display->getHeight() / 2, DisplayGDEW075T7::CENTER);
    _display->drawText(
        message,
        FONT_REGULAR,
        H_CENTER,
        _display->getHeight() / 2 + IMG_ERROR.height / 2 + 30,
        0,
        DisplayGDEW075T7::CENTER
    );
    _display->refresh();
}
