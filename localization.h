#include "config.h"

#ifndef PORTALCALENDAR_LOCALIZATION_H
#define PORTALCALENDAR_LOCALIZATION_H

#if defined(LOCALE_EN_US)

const char *I18N_MONTHS[] = {
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

const char *I18N_DAYS[] = {
    "SUNDAY",
    "MONDAY",
    "TUESDAY",
    "WEDNESDAY",
    "THURSDAY",
    "FRIDAY",
    "SATURDAY",
};

const char *I18N_DAYS_ABBREVIATIONS[] = {
    "SUN",
    "MON",
    "TUE",
    "WED",
    "THU",
    "FRI",
    "SAT",
};

#elif defined(LOCALE_DE_DE)

const char *I18N_MONTHS[] = {
    "JANUAR",
    "FEBRUAR",
    "MÄRZ",
    "APRIL",
    "MAI",
    "JUNI",
    "JULI",
    "AUGUST",
    "SEPTEMBER",
    "OKTOBER",
    "NOVEMBER",
    "DEZEMBER",
};

const char *I18N_DAYS[] = {
    "SONNTAG",
    "MONTAG",
    "DIENSTAG",
    "MITTWOCH",
    "DONNERSTAG",
    "FREITAG",
    "SAMSTAG",
};

const char *I18N_DAYS_ABBREVIATIONS[] = {
    "SO",
    "MO",
    "DI",
    "MI",
    "DO",
    "FR",
    "SA",
};

#else

#error No locale specified

#endif

#endif // PORTALCALENDAR_LOCALIZATION_H
