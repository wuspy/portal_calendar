#include "Display.h"
#include "global.h"
#include "Configuration.h"
#include "time_util.h"
#include "qrcodegen.h"

#include "resources/font/medium.h"
#include "resources/font/small.h"
#include "resources/font/chamber_number.h"

#include "resources/aperture_logo.h"
#include "resources/progress_bar.h"
#include "resources/wifi_64px.h"
#include "resources/error.h"

#include "resources/font/weather_frame.h"
#include "resources/weather_info_degree_symbol.h"
#include "resources/weather_info_percent_symbol.h"
#include "resources/weather_frame.h"
#include "resources/weather_frame_empty.h"
#include "resources/weather_cloudy.h"
#include "resources/weather_fog.h"
#include "resources/weather_thunderstorms.h"
#include "resources/weather_showers.h"
#include "resources/weather_snow.h"
#include "resources/weather_day_clear.h"
#include "resources/weather_night_clear.h"
#include "resources/weather_partly_cloudy_day.h"
#include "resources/weather_partly_cloudy_night.h"
#include "resources/weather_scattered_showers_day.h"
#include "resources/weather_scattered_showers_night.h"

#include "resources/cube_dispenser_on.h"
#include "resources/cube_dispenser_off.h"
#include "resources/cube_hazard_on.h"
#include "resources/cube_hazard_off.h"
#include "resources/pellet_hazard_on.h"
#include "resources/pellet_hazard_off.h"
#include "resources/pellet_catcher_on.h"
#include "resources/pellet_catcher_off.h"
#include "resources/water_hazard_on.h"
#include "resources/water_hazard_off.h"
#include "resources/fling_enter_on.h"
#include "resources/fling_enter_off.h"
#include "resources/fling_exit_on.h"
#include "resources/fling_exit_off.h"
#include "resources/turret_hazard_on.h"
#include "resources/turret_hazard_off.h"
#include "resources/dirty_water_on.h"
#include "resources/dirty_water_off.h"
#include "resources/cake_on.h"
#include "resources/cake_off.h"
#include "resources/bridge_shield_on.h"
#include "resources/bridge_shield_off.h"
#include "resources/cube_button_on.h"
#include "resources/cube_button_off.h"
#include "resources/faith_plate_on.h"
#include "resources/faith_plate_off.h"
#include "resources/laser_hazard_on.h"
#include "resources/laser_hazard_off.h"
#include "resources/laser_redirection_on.h"
#include "resources/laser_redirection_off.h"
#include "resources/laser_sensor_on.h"
#include "resources/laser_sensor_off.h"
#include "resources/light_bridge_on.h"
#include "resources/light_bridge_off.h"
#include "resources/player_button_on.h"
#include "resources/player_button_off.h"

using namespace qrcodegen;

const Image* CHAMBER_ICON_SETS[][10] = {{
    // P1 Chamber 1
    &IMG_CUBE_DISPENSER_OFF,    &IMG_CUBE_HAZARD_OFF,       &IMG_PELLET_HAZARD_OFF,     &IMG_PELLET_CATCHER_OFF,    &IMG_WATER_HAZARD_OFF,
    &IMG_FLING_ENTER_OFF,       &IMG_FLING_EXIT_OFF,        &IMG_TURRET_HAZARD_OFF,     &IMG_DIRTY_WATER_OFF,       &IMG_CAKE_OFF,
// }, {
//     // P1 Chamber 2
//     &IMG_CUBE_DISPENSER_OFF,    &IMG_CUBE_HAZARD_OFF,       &IMG_PELLET_HAZARD_OFF,     &IMG_PELLET_CATCHER_OFF,    &IMG_WATER_HAZARD_OFF,
//     &IMG_FLING_ENTER_OFF,       &IMG_FLING_EXIT_OFF,        &IMG_TURRET_HAZARD_OFF,     &IMG_DIRTY_WATER_OFF,       &IMG_CAKE_OFF,
// }, {
//     // P1 Chamber 3
//     &IMG_CUBE_DISPENSER_OFF,    &IMG_CUBE_HAZARD_OFF,       &IMG_PELLET_HAZARD_OFF,     &IMG_PELLET_CATCHER_OFF,    &IMG_WATER_HAZARD_OFF,
//     &IMG_FLING_ENTER_OFF,       &IMG_FLING_EXIT_OFF,        &IMG_TURRET_HAZARD_OFF,     &IMG_DIRTY_WATER_OFF,       &IMG_CAKE_OFF,
}, {
    // P1 Chamber 4
    &IMG_CUBE_DISPENSER_ON,     &IMG_CUBE_HAZARD_ON,        &IMG_PELLET_HAZARD_OFF,     &IMG_PELLET_CATCHER_OFF,    &IMG_WATER_HAZARD_OFF,
    &IMG_FLING_ENTER_OFF,       &IMG_FLING_EXIT_OFF,        &IMG_TURRET_HAZARD_OFF,     &IMG_DIRTY_WATER_OFF,       &IMG_CAKE_OFF,
}, {
    // P1 Chamber 5
    &IMG_CUBE_DISPENSER_OFF,    &IMG_CUBE_HAZARD_ON,        &IMG_PELLET_HAZARD_OFF,     &IMG_PELLET_CATCHER_OFF,    &IMG_WATER_HAZARD_OFF,
    &IMG_FLING_ENTER_OFF,       &IMG_FLING_EXIT_OFF,        &IMG_TURRET_HAZARD_OFF,     &IMG_DIRTY_WATER_OFF,       &IMG_CAKE_OFF,
}, {
    // P1 Chamber 6
    &IMG_CUBE_DISPENSER_OFF,    &IMG_CUBE_HAZARD_OFF,       &IMG_PELLET_HAZARD_ON,      &IMG_PELLET_CATCHER_ON,     &IMG_WATER_HAZARD_OFF,
    &IMG_FLING_ENTER_OFF,       &IMG_FLING_EXIT_OFF,        &IMG_TURRET_HAZARD_OFF,     &IMG_DIRTY_WATER_OFF,       &IMG_CAKE_OFF,
// }, {
//     // P1 Chamber 7
//     &IMG_CUBE_DISPENSER_OFF,    &IMG_CUBE_HAZARD_OFF,       &IMG_PELLET_HAZARD_ON,      &IMG_PELLET_CATCHER_ON,     &IMG_WATER_HAZARD_OFF,
//     &IMG_FLING_ENTER_OFF,       &IMG_FLING_EXIT_OFF,        &IMG_TURRET_HAZARD_OFF,     &IMG_DIRTY_WATER_OFF,       &IMG_CAKE_OFF,
}, {
    // P1 Chamber 8
    &IMG_CUBE_DISPENSER_OFF,    &IMG_CUBE_HAZARD_OFF,       &IMG_PELLET_HAZARD_ON,      &IMG_PELLET_CATCHER_ON,     &IMG_WATER_HAZARD_ON,
    &IMG_FLING_ENTER_OFF,       &IMG_FLING_EXIT_OFF,        &IMG_TURRET_HAZARD_OFF,     &IMG_DIRTY_WATER_ON,        &IMG_CAKE_OFF,
}, {
    // P1 Chamber 9
    &IMG_CUBE_DISPENSER_ON,     &IMG_CUBE_HAZARD_ON,        &IMG_PELLET_HAZARD_OFF,     &IMG_PELLET_CATCHER_OFF,    &IMG_WATER_HAZARD_OFF,
    &IMG_FLING_ENTER_OFF,       &IMG_FLING_EXIT_OFF,        &IMG_TURRET_HAZARD_OFF,     &IMG_DIRTY_WATER_OFF,       &IMG_CAKE_OFF,
}, {
    // P1 Chamber 10
    &IMG_CUBE_DISPENSER_OFF,    &IMG_CUBE_HAZARD_OFF,       &IMG_PELLET_HAZARD_OFF,     &IMG_PELLET_CATCHER_OFF,    &IMG_WATER_HAZARD_OFF,
    &IMG_FLING_ENTER_ON,        &IMG_FLING_EXIT_ON,         &IMG_TURRET_HAZARD_OFF,     &IMG_DIRTY_WATER_OFF,       &IMG_CAKE_OFF,
}, {
    // P1 Chamber 11
    &IMG_CUBE_DISPENSER_OFF,    &IMG_CUBE_HAZARD_OFF,       &IMG_PELLET_HAZARD_ON,      &IMG_PELLET_CATCHER_ON,     &IMG_WATER_HAZARD_ON,
    &IMG_FLING_ENTER_OFF,       &IMG_FLING_EXIT_OFF,        &IMG_TURRET_HAZARD_OFF,     &IMG_DIRTY_WATER_ON,        &IMG_CAKE_OFF,
}, {
    // P1 Chamber 12
    &IMG_CUBE_DISPENSER_ON,     &IMG_CUBE_HAZARD_ON,        &IMG_PELLET_HAZARD_OFF,     &IMG_PELLET_CATCHER_OFF,    &IMG_WATER_HAZARD_OFF,
    &IMG_FLING_ENTER_ON,        &IMG_FLING_EXIT_ON,         &IMG_TURRET_HAZARD_OFF,     &IMG_DIRTY_WATER_OFF,       &IMG_CAKE_OFF,
}, {
    // P1 Chamber 13
    &IMG_CUBE_DISPENSER_OFF,    &IMG_CUBE_HAZARD_ON,        &IMG_PELLET_HAZARD_ON,      &IMG_PELLET_CATCHER_ON,     &IMG_WATER_HAZARD_OFF,
    &IMG_FLING_ENTER_OFF,       &IMG_FLING_EXIT_OFF,        &IMG_TURRET_HAZARD_OFF,     &IMG_DIRTY_WATER_OFF,       &IMG_CAKE_OFF,
}, {
    // P1 Chamber 14
    &IMG_CUBE_DISPENSER_OFF,    &IMG_CUBE_HAZARD_ON,        &IMG_PELLET_HAZARD_ON,      &IMG_PELLET_CATCHER_ON,     &IMG_WATER_HAZARD_ON,
    &IMG_FLING_ENTER_ON,        &IMG_FLING_EXIT_ON,         &IMG_TURRET_HAZARD_OFF,     &IMG_DIRTY_WATER_ON,        &IMG_CAKE_OFF,
}, {
    // P1 Chamber 15
    &IMG_CUBE_DISPENSER_OFF,    &IMG_CUBE_HAZARD_OFF,       &IMG_PELLET_HAZARD_ON,      &IMG_PELLET_CATCHER_ON,     &IMG_WATER_HAZARD_ON,
    &IMG_FLING_ENTER_ON,        &IMG_FLING_EXIT_ON,         &IMG_TURRET_HAZARD_OFF,     &IMG_DIRTY_WATER_ON,        &IMG_CAKE_OFF,
}, {
    // P1 Chamber 16
    &IMG_CUBE_DISPENSER_ON,     &IMG_CUBE_HAZARD_ON,        &IMG_PELLET_HAZARD_OFF,     &IMG_PELLET_CATCHER_OFF,    &IMG_WATER_HAZARD_OFF,
    &IMG_FLING_ENTER_OFF,       &IMG_FLING_EXIT_OFF,        &IMG_TURRET_HAZARD_ON,      &IMG_DIRTY_WATER_OFF,       &IMG_CAKE_OFF,
}, {
    // P1 Chamber 17
    &IMG_CUBE_DISPENSER_ON,     &IMG_CUBE_HAZARD_ON,        &IMG_PELLET_HAZARD_ON,      &IMG_PELLET_CATCHER_ON,     &IMG_WATER_HAZARD_OFF,
    &IMG_FLING_ENTER_OFF,       &IMG_FLING_EXIT_OFF,        &IMG_TURRET_HAZARD_OFF,     &IMG_DIRTY_WATER_OFF,       &IMG_CAKE_OFF,
}, {
    // P1 Chamber 18
    &IMG_CUBE_DISPENSER_ON,     &IMG_CUBE_HAZARD_ON,        &IMG_PELLET_HAZARD_ON,      &IMG_PELLET_CATCHER_ON,     &IMG_WATER_HAZARD_ON,
    &IMG_FLING_ENTER_ON,        &IMG_FLING_EXIT_ON,         &IMG_TURRET_HAZARD_ON,      &IMG_DIRTY_WATER_ON,        &IMG_CAKE_OFF,
}, {
    // P1 Chamber 19
    &IMG_CUBE_DISPENSER_OFF,    &IMG_CUBE_HAZARD_OFF,       &IMG_PELLET_HAZARD_ON,      &IMG_PELLET_CATCHER_ON,     &IMG_WATER_HAZARD_ON,
    &IMG_FLING_ENTER_OFF,       &IMG_FLING_EXIT_OFF,        &IMG_TURRET_HAZARD_OFF,     &IMG_DIRTY_WATER_ON,        &IMG_CAKE_ON,
}, {
    // P2 The Cold Boot Chamber 1
    &IMG_LASER_SENSOR_ON,       &IMG_LASER_REDIRECTION_OFF, &IMG_CUBE_DISPENSER_OFF,    &IMG_CUBE_BUTTON_OFF,       &IMG_CUBE_HAZARD_OFF,
    &IMG_PLAYER_BUTTON_OFF,     &IMG_WATER_HAZARD_OFF,      &IMG_TURRET_HAZARD_OFF,     &IMG_LASER_HAZARD_OFF,      &IMG_DIRTY_WATER_OFF,
}, {
    // P2 The Cold Boot Chamber 2
    &IMG_CUBE_DISPENSER_ON,     &IMG_CUBE_BUTTON_ON,        &IMG_CUBE_HAZARD_ON,        &IMG_PLAYER_BUTTON_OFF,     &IMG_WATER_HAZARD_OFF,
    &IMG_LASER_SENSOR_ON,       &IMG_LASER_REDIRECTION_ON,  &IMG_TURRET_HAZARD_OFF,     &IMG_LASER_HAZARD_OFF,      &IMG_DIRTY_WATER_OFF,
}, {
    // P2 The Cold Boot Chamber 3
    &IMG_CUBE_DISPENSER_OFF,    &IMG_CUBE_BUTTON_OFF,       &IMG_CUBE_HAZARD_OFF,       &IMG_PLAYER_BUTTON_OFF,     &IMG_WATER_HAZARD_OFF,
    &IMG_LASER_SENSOR_ON,       &IMG_LASER_REDIRECTION_ON,  &IMG_TURRET_HAZARD_OFF,     &IMG_LASER_HAZARD_OFF,      &IMG_DIRTY_WATER_OFF,
}, {
    // P2 The Cold Boot Chamber 4
    &IMG_CUBE_DISPENSER_ON,     &IMG_CUBE_BUTTON_ON,        &IMG_CUBE_HAZARD_ON,        &IMG_PLAYER_BUTTON_OFF,     &IMG_WATER_HAZARD_ON,
    &IMG_LASER_SENSOR_ON,       &IMG_LASER_REDIRECTION_OFF, &IMG_TURRET_HAZARD_OFF,     &IMG_LASER_HAZARD_OFF,      &IMG_DIRTY_WATER_OFF,
}, {
    // P2 The Cold Boot Chamber 5
    &IMG_CUBE_DISPENSER_ON,     &IMG_CUBE_BUTTON_ON,        &IMG_CUBE_HAZARD_ON,        &IMG_PLAYER_BUTTON_OFF,     &IMG_WATER_HAZARD_ON,
    &IMG_FLING_ENTER_OFF,       &IMG_FLING_EXIT_OFF,        &IMG_FAITH_PLATE_ON,        &IMG_TURRET_HAZARD_OFF,     &IMG_DIRTY_WATER_OFF,
// }, {
//     // P2 The Cold Boot Chamber 6
//     &IMG_CUBE_DISPENSER_ON,     &IMG_CUBE_BUTTON_ON,        &IMG_CUBE_HAZARD_ON,        &IMG_PLAYER_BUTTON_OFF,     &IMG_WATER_HAZARD_ON,
//     &IMG_FLING_ENTER_OFF,       &IMG_FLING_EXIT_OFF,        &IMG_FAITH_PLATE_ON,        &IMG_TURRET_HAZARD_OFF,     &IMG_DIRTY_WATER_OFF,
}, {
    // P2 The Cold Boot Chamber 7
    &IMG_CUBE_DISPENSER_ON,     &IMG_CUBE_BUTTON_ON,        &IMG_CUBE_HAZARD_ON,        &IMG_WATER_HAZARD_OFF,      &IMG_FLING_ENTER_ON,
    &IMG_FLING_EXIT_ON,         &IMG_LASER_SENSOR_ON,       &IMG_LASER_REDIRECTION_OFF, &IMG_TURRET_HAZARD_OFF,     &IMG_DIRTY_WATER_OFF,
}, {
    // P2 The Cold Boot Chamber 8
    &IMG_CUBE_DISPENSER_ON,     &IMG_CUBE_BUTTON_ON,        &IMG_CUBE_HAZARD_ON,        &IMG_WATER_HAZARD_OFF,      &IMG_FLING_ENTER_OFF,
    &IMG_FLING_EXIT_OFF,        &IMG_LASER_SENSOR_ON,       &IMG_LASER_REDIRECTION_ON,  &IMG_TURRET_HAZARD_OFF,     &IMG_DIRTY_WATER_OFF,
}, {
    // P2 The Return Chamber 9
    &IMG_CUBE_DISPENSER_ON,     &IMG_CUBE_BUTTON_OFF,       &IMG_CUBE_HAZARD_ON,        &IMG_PLAYER_BUTTON_OFF,     &IMG_WATER_HAZARD_OFF,
    &IMG_LASER_SENSOR_ON,       &IMG_LASER_REDIRECTION_ON,  &IMG_FAITH_PLATE_ON,        &IMG_TURRET_HAZARD_OFF,     &IMG_DIRTY_WATER_OFF,
}, {
    // P2 The Return Chamber 10
    &IMG_CUBE_DISPENSER_ON,     &IMG_CUBE_BUTTON_ON,        &IMG_CUBE_HAZARD_ON,        &IMG_WATER_HAZARD_OFF,      &IMG_LASER_SENSOR_ON,
    &IMG_LASER_REDIRECTION_ON,  &IMG_FAITH_PLATE_ON,        &IMG_FLING_ENTER_ON,        &IMG_FLING_EXIT_ON,         &IMG_DIRTY_WATER_OFF,
}, {
    // P2 The Return Chamber 11
    &IMG_CUBE_DISPENSER_ON,     &IMG_CUBE_BUTTON_ON,        &IMG_CUBE_HAZARD_ON,        &IMG_PLAYER_BUTTON_OFF,     &IMG_WATER_HAZARD_ON,
    &IMG_LIGHT_BRIDGE_ON,       &IMG_TURRET_HAZARD_OFF,     &IMG_BRIDGE_SHIELD_OFF,     &IMG_LASER_REDIRECTION_OFF, &IMG_DIRTY_WATER_OFF,
// }, {
//     // P2 The Return Chamber 12
//     &IMG_CUBE_DISPENSER_ON,     &IMG_CUBE_BUTTON_ON,        &IMG_CUBE_HAZARD_ON,        &IMG_PLAYER_BUTTON_OFF,     &IMG_WATER_HAZARD_ON,
//     &IMG_LIGHT_BRIDGE_ON,       &IMG_TURRET_HAZARD_OFF,     &IMG_BRIDGE_SHIELD_OFF,     &IMG_LASER_REDIRECTION_OFF, &IMG_DIRTY_WATER_OFF,
}, {
    // P2 The Return Chamber 13
    &IMG_CUBE_DISPENSER_OFF,    &IMG_CUBE_BUTTON_ON,        &IMG_CUBE_HAZARD_ON,        &IMG_PLAYER_BUTTON_OFF,     &IMG_WATER_HAZARD_OFF,
    &IMG_LIGHT_BRIDGE_OFF,      &IMG_TURRET_HAZARD_ON,      &IMG_BRIDGE_SHIELD_OFF,     &IMG_LASER_HAZARD_OFF,      &IMG_DIRTY_WATER_OFF,
// }, {
//     // P2 The Return Chamber 14
//     &IMG_CUBE_DISPENSER_OFF,    &IMG_CUBE_BUTTON_OFF,       &IMG_CUBE_HAZARD_OFF,       &IMG_PLAYER_BUTTON_OFF,     &IMG_WATER_HAZARD_OFF,
//     &IMG_LASER_REDIRECTION_ON,  &IMG_LASER_SENSOR_ON,       &IMG_TURRET_HAZARD_OFF,     &IMG_LASER_HAZARD_OFF,      &IMG_DIRTY_WATER_OFF,
}, {
    // P2 The Return Chamber 15
    &IMG_CUBE_DISPENSER_OFF,    &IMG_CUBE_BUTTON_ON,        &IMG_CUBE_HAZARD_OFF,       &IMG_PLAYER_BUTTON_OFF,     &IMG_WATER_HAZARD_OFF,
    &IMG_LIGHT_BRIDGE_ON,       &IMG_TURRET_HAZARD_ON,      &IMG_BRIDGE_SHIELD_ON,      &IMG_FAITH_PLATE_ON,        &IMG_DIRTY_WATER_OFF,
}, {
    // P2 The Return Chamber 16
    &IMG_CUBE_DISPENSER_OFF,    &IMG_CUBE_BUTTON_ON,        &IMG_CUBE_HAZARD_OFF,       &IMG_PLAYER_BUTTON_ON,      &IMG_WATER_HAZARD_OFF,
    &IMG_LASER_REDIRECTION_ON,  &IMG_LASER_SENSOR_ON,       &IMG_TURRET_HAZARD_ON,      &IMG_LASER_HAZARD_ON,       &IMG_DIRTY_WATER_OFF,
// }, {
//     // P2 The Return Chamber 17
//     &IMG_CUBE_DISPENSER_OFF,    &IMG_CUBE_BUTTON_ON,        &IMG_CUBE_HAZARD_OFF,       &IMG_PLAYER_BUTTON_OFF,      &IMG_WATER_HAZARD_OFF,
//     &IMG_LIGHT_BRIDGE_ON,       &IMG_LASER_SENSOR_ON,       &IMG_TURRET_HAZARD_OFF,     &IMG_BRIDGE_SHIELD_OFF,      &IMG_DIRTY_WATER_OFF,
}, {
    // P2 The Surprise Chamber 18
    &IMG_CUBE_DISPENSER_ON,     &IMG_CUBE_BUTTON_OFF,       &IMG_CUBE_HAZARD_ON,        &IMG_WATER_HAZARD_ON,       &IMG_LIGHT_BRIDGE_ON,
    &IMG_LASER_SENSOR_ON,       &IMG_LASER_REDIRECTION_ON,  &IMG_TURRET_HAZARD_ON,      &IMG_BRIDGE_SHIELD_ON,      &IMG_LASER_HAZARD_ON,
}, {
    // P2 The Surprise Chamber 19
    &IMG_CUBE_DISPENSER_OFF,    &IMG_CUBE_BUTTON_OFF,       &IMG_CUBE_HAZARD_OFF,       &IMG_PLAYER_BUTTON_OFF,     &IMG_LASER_SENSOR_ON,
    &IMG_LASER_REDIRECTION_ON,  &IMG_FAITH_PLATE_ON,        &IMG_TURRET_HAZARD_ON,      &IMG_LASER_HAZARD_ON,       &IMG_DIRTY_WATER_OFF,
// }, {
//     // P2 The Surprise Chamber 20
//     &IMG_FLING_ENTER_OFF,       &IMG_FLING_EXIT_OFF,        &IMG_LIGHT_BRIDGE_OFF,      &IMG_PLAYER_BUTTON_OFF,     &IMG_LASER_SENSOR_ON,
//     &IMG_LASER_REDIRECTION_ON,  &IMG_FAITH_PLATE_OFF,       &IMG_TURRET_HAZARD_OFF,     &IMG_LASER_HAZARD_OFF,      &IMG_DIRTY_WATER_OFF,
// }, {
//     // P2 The Surprise Chamber 21
//     &IMG_LIGHT_BRIDGE_ON,       &IMG_CUBE_DISPENSER_ON,     &IMG_CUBE_BUTTON_ON,        &IMG_CUBE_HAZARD_ON,        &IMG_LASER_SENSOR_OFF,
//     &IMG_LASER_REDIRECTION_OFF, &IMG_TURRET_HAZARD_OFF,     &IMG_LASER_HAZARD_OFF,      &IMG_WATER_HAZARD_ON,       &IMG_DIRTY_WATER_OFF,
}};

static_assert(
    sizeof(CHAMBER_ICON_SETS) / sizeof(Image*[10]) == 31,
    "There must be 31 sets of chamber icons"
);

#define ICON_SIZE 64
#define ICON_SPACING 9
#define LEFT 82
#define RIGHT (LEFT + ICON_SIZE * 5 + ICON_SPACING * 4)
#define WIDTH (RIGHT - LEFT)
#define ICON_TOP 550
// Part of the physical screen is covered by the bezel to change the aspect ratio from 15:9 to 16:9,
// this marks the horizontal centerline of the visible area
#define H_CENTER 225

DisplayClass Display;

DisplayClass::~DisplayClass()
{
    if (_display) {
        delete _display;
        _display = nullptr;
    }
}

void DisplayClass::init()
{
    if (!_display) {
        _display = new DisplayGDEW075T7(SPI_BUS, CLK_PIN, DIN_PIN, CS_PIN, RESET_PIN, DC_PIN, BUSY_PIN);
        _display->setRotation(DisplayGDEW075T7::ROTATION_270);
        _display->setAlpha(DisplayGDEW075T7::WHITE);
    } else {
        _display->clear();
    }
}

void DisplayClass::update(const tm *now, const Locale& locale, bool showWeather)
{
    init();

    const int year = now->tm_year + 1900;
    const int daysInMonth = getDaysInMonth(now->tm_mon, year);

    drawStandardSeparators();
    drawApertureLogo();
    drawChamberNumber(now->tm_mday, daysInMonth);

    if (Config.getShowDay()) {
        // Day name
        _display->drawText(locale.days[now->tm_wday], FONT_MEDIUM, RIGHT, 394, DisplayGDEW075T7::TOP_RIGHT);
    }

    if (Config.getShowMonth()) {
        // Month name
        _display->drawText(locale.months[now->tm_mon], FONT_MEDIUM, LEFT, 14);
    }

    if (Config.getShowYear()) {
        // Year
        char buffer[5];
        sprintf(buffer, "%d", year);
        _display->drawText(buffer, FONT_MEDIUM, RIGHT, 14, DisplayGDEW075T7::TOP_RIGHT);
    }

    // Progress bar
    _display->drawImage(IMG_PROGRESS_BAR, LEFT, 438);
    int32_t progressWidth = IMG_PROGRESS_BAR.width * now->tm_mday / daysInMonth;
    _display->fillRect(LEFT + progressWidth, 438, IMG_PROGRESS_BAR.width - progressWidth, IMG_PROGRESS_BAR.height, DisplayGDEW075T7::WHITE);

    if (Config.getWeatherEnabled() && showWeather) {
        // Weather
        WeatherDisplayType displayType = Config.getWeatherDisplayType();
        if (displayType == WeatherDisplayType::FORECAST_5_DAY) {
            DailyWeather weather[5];
            get5DayWeather(now->tm_mon, now->tm_mday, year, weather);

            for (int i = 0; i < 5; ++i) {
                drawDailyWeather(weather[i], i, locale);
            }
        } else if (displayType == WeatherDisplayType::FORECAST_12_HOUR) {
            WeatherEntry weather[5];
            getTodaysWeather(now->tm_mon, now->tm_mday, weather);

            for (int i = 0; i < 5; ++i) {
                drawWeatherEntry(weather[i], i);
            }
        }
    } else {
        // Chamber icons
        if (now->tm_mon == 1 && now->tm_mday == 29) {
            // Special icon set for leap day
            for (int i = 0; i < 8; ++i) {
                drawChamberIcon(IMG_TURRET_HAZARD_ON, i % 5, i / 5);
            }
        } else if (now->tm_mday <= 31) {
            for (int i = 0; i < 10; ++i) {
                drawChamberIcon(*CHAMBER_ICON_SETS[now->tm_mday - 1][i], i % 5, i / 5);
            }
        }
    }

    _display->refresh();
}

const Image* DisplayClass::getWeatherConditionIcon(WeatherCondition condition, bool day)
{
    switch (condition) {
        case WeatherCondition::CLEAR:
        case WeatherCondition::FEW_CLOUDS:
            return day ? &IMG_WEATHER_DAY_CLEAR : &IMG_WEATHER_NIGHT_CLEAR;
        case WeatherCondition::SCATTERED_CLOUDS:
        case WeatherCondition::BROKEN_CLOUDS:
            return day ? &IMG_WEATHER_PARTLY_CLOUDY_DAY : &IMG_WEATHER_PARTLY_CLOUDY_NIGHT;
        case WeatherCondition::OVERCAST_CLOUDS:
            return &IMG_WEATHER_CLOUDY;
        case WeatherCondition::SCATTERED_SHOWERS:
            return day ? &IMG_WEATHER_SCATTERED_SHOWERS_DAY : &IMG_WEATHER_SCATTERED_SHOWERS_NIGHT;
        case WeatherCondition::SHOWERS:
            return &IMG_WEATHER_SHOWERS;
        case WeatherCondition::THUNDERSTORM:
            return &IMG_WEATHER_THUNDERSTORMS;
        case WeatherCondition::FOG:
            return &IMG_WEATHER_FOG;
        case WeatherCondition::FREEZING_RAIN:
        case WeatherCondition::SNOW:
            return &IMG_WEATHER_SNOW;
        default:
            return nullptr;
    }
}

void DisplayClass::drawWeatherInfoText(const char* text, const Image* symbol, int32_t x, int32_t y)
{
    if (symbol) {
        uint32_t textWidth = _display->measureText(text, FONT_SMALL);
        // The extra /2 gives less weight to the symbol so the text appears more centered
        x -= (textWidth + symbol->width / 2) / 2;
        _display->drawText(text, FONT_SMALL, x, y);
        _display->drawImage(*symbol, x + textWidth, y);
    } else {
        _display->drawText(text, FONT_SMALL, x, y, DisplayGDEW075T7::TOP_CENTER);
    }
}

void DisplayClass::drawDailyWeather(const DailyWeather& weather, int32_t x, const Locale& locale)
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
    _display->drawText(locale.dayAbbreviations[weather.wday], FONT_WEATHER_FRAME, x + 5, ICON_TOP);
    sprintf(text, "%d", weather.mday);
    _display->drawText(text, FONT_WEATHER_FRAME, x + 64 - 5, ICON_TOP, DisplayGDEW075T7::TOP_RIGHT);
    _display->setAlpha(DisplayGDEW075T7::WHITE);

    // Draw high temp
    sprintf(text, "%d", weather.highTemp);
    drawWeatherInfoText(text, &IMG_WEATHER_INFO_DEGREE_SYMBOL, x + 32, ICON_TOP + 83);

    // Draw low temp
    sprintf(text, "%d", weather.lowTemp);
    drawWeatherInfoText(text, &IMG_WEATHER_INFO_DEGREE_SYMBOL, x + 32, ICON_TOP + 108);
}

void DisplayClass::drawWeatherEntry(const WeatherEntry& weather, int32_t x)
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
    if (Config.getShow24HourTime()) {
        sprintf(text, "%02d:%02d", weather.hour, weather.minute);
    } else {
        int8_t hour12 = weather.hour % 12;
        if (hour12 == 0) {
            hour12 = 12;
        }
        sprintf(text, "%d:%02d %s", hour12, weather.minute, weather.hour > 11 ? "PM" : "AM");
    }

    _display->setAlpha(DisplayGDEW075T7::BLACK);
    _display->drawText(text, FONT_WEATHER_FRAME, x + 32, ICON_TOP, DisplayGDEW075T7::TOP_CENTER);
    _display->setAlpha(DisplayGDEW075T7::WHITE);

    // Draw temperature
    sprintf(text, "%d", weather.temp);
    drawWeatherInfoText(text, &IMG_WEATHER_INFO_DEGREE_SYMBOL, x + 32, ICON_TOP + 83);

    // Draw secondary info
    WeatherSecondaryInfo secondaryInfo = Config.getWeatherSecondaryInfo();
    if (secondaryInfo == WeatherSecondaryInfo::POP) {
        sprintf(text, "%d", weather.pop);
        drawWeatherInfoText(text, &IMG_WEATHER_INFO_PERCENT_SYMBOL, x + 32, ICON_TOP + 108);
    } else if (secondaryInfo == WeatherSecondaryInfo::HUMIDITY) {
        sprintf(text, "%d", weather.humidity);
        drawWeatherInfoText(text, &IMG_WEATHER_INFO_PERCENT_SYMBOL, x + 32, ICON_TOP + 108);
    }
}

#ifdef DEBUG

void DisplayClass::testChamberIcons()
{
    const Image* allIcons[] = {
        &IMG_CUBE_DISPENSER_ON,     &IMG_CUBE_HAZARD_ON,        &IMG_PELLET_HAZARD_ON,      &IMG_PELLET_CATCHER_ON,     &IMG_WATER_HAZARD_ON,
        &IMG_CUBE_DISPENSER_OFF,    &IMG_CUBE_HAZARD_OFF,       &IMG_PELLET_HAZARD_OFF,     &IMG_PELLET_CATCHER_OFF,    &IMG_WATER_HAZARD_OFF,
        &IMG_FLING_ENTER_ON,        &IMG_FLING_EXIT_ON,         &IMG_TURRET_HAZARD_ON,      &IMG_DIRTY_WATER_ON,        &IMG_CAKE_ON,
        &IMG_FLING_ENTER_OFF,       &IMG_FLING_EXIT_OFF,        &IMG_TURRET_HAZARD_OFF,     &IMG_DIRTY_WATER_OFF,       &IMG_CAKE_OFF,
        &IMG_CUBE_BUTTON_ON,        &IMG_PLAYER_BUTTON_ON,      &IMG_LIGHT_BRIDGE_ON,       &IMG_FAITH_PLATE_ON,        &IMG_LASER_SENSOR_ON,
        &IMG_CUBE_BUTTON_OFF,       &IMG_PLAYER_BUTTON_OFF,     &IMG_LIGHT_BRIDGE_OFF,      &IMG_FAITH_PLATE_OFF,       &IMG_LASER_SENSOR_OFF,
        &IMG_LASER_REDIRECTION_ON,  &IMG_BRIDGE_SHIELD_ON,      &IMG_LASER_HAZARD_ON,       nullptr,                    nullptr,
        &IMG_LASER_REDIRECTION_OFF, &IMG_BRIDGE_SHIELD_OFF,     &IMG_LASER_HAZARD_OFF,
    };

    init();
    for (int i = 0; i < sizeof(allIcons) / sizeof(Image*); ++i) {
        if (allIcons[i] != nullptr) {
            _display->drawImage(
                *allIcons[i],
                20 + (i % 5) * (ICON_SIZE + ICON_SPACING),
                20 + (i / 5) * (ICON_SIZE + ICON_SPACING)
            );
        }
    }
    _display->refresh();
}

#endif // DEBUG

void DisplayClass::drawChamberIcon(const Image& icon, int32_t x, int32_t y)
{
    _display->drawImage(
        icon,
        LEFT + x * (ICON_SIZE + ICON_SPACING),
        ICON_TOP + y * (ICON_SIZE + ICON_SPACING)
    );
}

void DisplayClass::drawStandardSeparators()
{
    _display->drawHLine(LEFT, 50, WIDTH, 2, DisplayGDEW075T7::BLACK, DisplayGDEW075T7::TOP_LEFT);
    _display->drawHLine(LEFT, 430, WIDTH, 2, DisplayGDEW075T7::BLACK, DisplayGDEW075T7::TOP_LEFT);
    _display->drawHLine(LEFT, 538, WIDTH, 2, DisplayGDEW075T7::BLACK, DisplayGDEW075T7::TOP_LEFT);
}

void DisplayClass::drawChamberNumber(int number, int total)
{
    char buffer[10];
    
    // BIG date
    sprintf(buffer, "%02d", number);
    _display->drawText(buffer, FONT_CHAMBER_NUMBER, LEFT, 16, DisplayGDEW075T7::TOP_LEFT, 10);

    // Small "XX/XX" date
    sprintf(buffer, "%02d/%02d", number, total);
    _display->drawText(buffer, FONT_MEDIUM, LEFT, 394);    
}

void DisplayClass::drawApertureLogo()
{
    _display->drawImage(IMG_APERTURE_LOGO, LEFT, 740);
}

void DisplayClass::error(String message, bool willRetry)
{
    init();
    const int32_t y = _display->getHeight() - _display->getHeight() / 1.618;
    _display->drawImage(IMG_ERROR, H_CENTER, y, DisplayGDEW075T7::BOTTOM_CENTER);
    _display->drawMultilineText(
        message,
        FONT_SMALL,
        H_CENTER,
        y + FONT_SMALL.ascent + FONT_SMALL.descent,
        408,
        DisplayGDEW075T7::TOP_CENTER
    );

    if (willRetry) {
        _display->drawMultilineText(
            "Will try again in 1 hour. Or, press the RESET button on the back of the device to retry now.",
            FONT_SMALL,
            H_CENTER,
            _display->getHeight() - 12,
            400,
            DisplayGDEW075T7::BOTTOM_CENTER
        );
    }

    _display->refresh();
}

void DisplayClass::showWelcomeScreen()
{
    init();
    drawStandardSeparators();
    drawApertureLogo();
    drawChamberNumber(0, 0);

    _display->drawText("WELCOME!", FONT_MEDIUM, LEFT, 502);
    _display->drawMultilineText(
        "Connect to USB power, then press the RESET button on the back to begin setup.",
        FONT_SMALL,
        LEFT,
        544,
        360
    );

    _display->refresh();
}

void DisplayClass::showConfigServerScreen(String ssid, String password, String hostname, String connectedWifiName)
{
    const int32_t QR_SCALE = 7;
    char str[120];
    sprintf(&str[0], "WIFI:T:WPA;S:%s;P:%s;;", ssid.c_str(), password.c_str());
    QrCode qrCode = QrCode::encodeText(&str[0], QrCode::Ecc::ECC_HIGH);

    init();
    drawStandardSeparators();
    drawApertureLogo();

    _display->drawQrCode(qrCode, LEFT, 75, QR_SCALE);
    const uint32_t qrX = LEFT + qrCode.getSize() * QR_SCALE / 2;
    const uint32_t qrY = 75 + qrCode.getSize() * QR_SCALE / 2;
    _display->fillRect(qrX, qrY, 77, 77, DisplayGDEW075T7::WHITE, DisplayGDEW075T7::CENTER);
    _display->drawImage(IMG_WIFI_64PX, qrX, qrY, DisplayGDEW075T7::CENTER);
    _display->drawMultilineText(
        "Wi-Fi Name: " + ssid + "\n"
        "Password: " + password,
        FONT_SMALL,
        qrX,
        75 + qrCode.getSize() * QR_SCALE + 12,
        0,
        DisplayGDEW075T7::TOP_CENTER
    );

    _display->drawText("SETUP", FONT_MEDIUM, LEFT, 502);
    _display->drawMultilineText(
        connectedWifiName.isEmpty()
            ? "Connect to this Wi-Fi network, then open a web browser and go to:\n\n"
              "http://" + hostname + ".local"
            : "Connect either to this Wi-Fi network, or the network '" + connectedWifiName + "', "
              "then open a web browser and go to:\n\n"
              "http://" + hostname + ".local",
        FONT_SMALL,
        LEFT,
        544,
        360
    );

    _display->refresh();
    delete _display;
    _display = nullptr;
}

#ifdef DEV_WEBSERVER

void DisplayClass::showDevWebserverScreen(String ssid, IPAddress localIp)
{
    init();
    _display->drawMultilineText(
        "Dev webserver running.\n"
        "SSID: " + ssid + "\n"
        "IP: " + localIp.toString(),
        FONT_SMALL,
        H_CENTER,
        _display->getHeight() / 2,
        _display->getWidth(),
        DisplayGDEW075T7::TOP_CENTER
    );
    _display->refresh();
    delete _display;
    _display = nullptr;
}

#endif // DEV_WEBSERVER
