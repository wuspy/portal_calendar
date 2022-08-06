#include "Display.h"
#include "config.h"

#include "resource/font/Univers 65 Bold Regular_28px.h"
#include "resource/font/Univers LT 49 Light Ultra Condensed_400px.h"

#include "resource/aperture_logo.h"
#include "resource/progress_bar.h"
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

#define LEFT 110
#define RIGHT 470
#define WIDTH RIGHT - LEFT
#define ICON_SIZE 64
#define ICON_SPACING 9

#define SMALL_FONT FONT_UNIVERS_65_BOLD_REGULAR_28PX
#define LARGE_NUMBER_FONT FONT_UNIVERS_LT_49_LIGHT_ULTRA_CONDENSED_400PX

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

int getDaysInMonth(int month, int year)
{
    switch (month) {
        case 2:
            return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0) ? 29 : 28;
        case 4:
        case 6:
        case 9:
        case 11:
            return 30;
        default:
            return 31;
    }
}

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
        _display->setRotation(DisplayGDEW075T7::ROTATION_90);
        _display->setAlpha(DisplayGDEW075T7::WHITE);
    }
}

void Display::update(const tm *now)
{
    init();
    int daysInMonth = getDaysInMonth(now->tm_mon + 1, now->tm_year + 1900);

    // Static lines
    _display->drawHLine(LEFT, 50, WIDTH, 2, DisplayGDEW075T7::BLACK, DisplayGDEW075T7::TOP_LEFT);
    _display->drawHLine(LEFT, 430, WIDTH, 2, DisplayGDEW075T7::BLACK, DisplayGDEW075T7::TOP_LEFT);
    _display->drawHLine(LEFT, 538, WIDTH, 2, DisplayGDEW075T7::BLACK, DisplayGDEW075T7::TOP_LEFT);

    // Aperture logo
    _display->drawImage(IMG_APERTURE_LOGO, LEFT, 740);

    // BIG date
    char bigDay[3];
    sprintf(bigDay, "%02d", now->tm_mday);
    _display->drawText(bigDay, LARGE_NUMBER_FONT, LEFT, 16, 10);

    // Small "XX/XX" date
    char smallDay[6];
    sprintf(smallDay, "%02d/%02d", now->tm_mday, daysInMonth);
    _display->drawText(smallDay, SMALL_FONT, LEFT, 394);

    #ifdef SHOW_DAY
    // Day name
    _display->drawText(DAYS[now->tm_wday], SMALL_FONT, RIGHT, 394, 0, DisplayGDEW075T7::TOP_RIGHT);
    #endif

    #ifdef SHOW_MONTH
    // Month name
    _display->drawText(MONTHS[now->tm_mon], SMALL_FONT, LEFT, 14, 0);
    #endif

    #ifdef SHOW_YEAR
    // Year
    char year[5];
    sprintf(year, "%d", now->tm_year + 1900);
    _display->drawText(year, SMALL_FONT, RIGHT, 14, 0, DisplayGDEW075T7::TOP_RIGHT);
    #endif

    // Progress bar
    _display->drawImage(IMG_PROGRESS_BAR, LEFT, 438);
    int32_t progressWidth = IMG_PROGRESS_BAR.width * now->tm_mday / daysInMonth;
    _display->fillRect(LEFT + progressWidth, 438, IMG_PROGRESS_BAR.width - progressWidth, IMG_PROGRESS_BAR.height, DisplayGDEW075T7::WHITE);

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
    
    _display->refresh();
}

void Display::drawIcon(const Image icon, int32_t x, int32_t y)
{
    _display->drawImage(
        icon,
        LEFT + x * (ICON_SIZE + ICON_SPACING),
        550 + y * (ICON_SIZE + ICON_SPACING)
    );
}

void Display::error(String message)
{
    init();
    _display->drawText(
        message,
        SMALL_FONT,
        _display->getWidth() / 2,
        _display->getHeight() / 2,
        0,
        DisplayGDEW075T7::CENTER
    );
    _display->refresh();
}
