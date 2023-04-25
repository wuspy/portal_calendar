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

#elif defined(LOCALE_ES_ES)

const char *I18N_MONTHS[] = {
    "ENERO",
    "FEBRERO",
    "MARZO",
    "ABRIL",
    "MAYO",
    "JUNIO",
    "JULIO",
    "AGOSTO",
    "SEPTIEMBRE",
    "OCTUBRE",
    "NOVIEMBRE",
    "DICIEMBRE",
};

const char *I18N_DAYS[] = {
    "DOMINGO",
    "LUNES",
    "MARTES",
    "MIÉRCOLES",
    "JUEVES",
    "VIERNES",
    "SÁBADO",
};

const char *I18N_DAYS_ABBREVIATIONS[] = {
    "DOM",
    "LUN",
    "MAR",
    "MIÉ",
    "JUE",
    "VIE",
    "SÁB",
};

#elif defined(LOCALE_FR_FR)

const char *I18N_MONTHS[] = {
    "JANVIER",
    "FÉVRIER",
    "MARS",
    "AVRIL",
    "MAI",
    "JUIN",
    "JUILLET",
    "AOÛT",
    "SEPTEMBRE",
    "OCTOBRE",
    "NOVEMBRE",
    "DÉCEMBRE",
};

const char *I18N_DAYS[] = {
    "DIMANCHE",
    "LUNDI",
    "MARDI",
    "MERCREDI",
    "JEUDI",
    "VENDREDI",
    "SAMEDI",
};

const char *I18N_DAYS_ABBREVIATIONS[] = {
    "DIM.",
    "LUN.",
    "MAR.",
    "MER.",
    "JEU.",
    "VEN.",
    "SAM.",
};

#elif defined(LOCALE_NL_NL)

const char *I18N_MONTHS[] = {
    "JANUARI",
    "FEBRUARI",
    "MAART",
    "APRIL",
    "MEI",
    "JUNI",
    "JULI",
    "AUGUSTUS",
    "SEPTEMBER",
    "OKTOBER",
    "NOVEMBER",
    "DECEMBER",
};

const char *I18N_DAYS[] = {
    "ZONDAG",
    "MAANDAG",
    "DINSDAG",
    "WOENSDAG",
    "DONDERDAG",
    "VRIJDAG",
    "ZATERDAG",
};

const char *I18N_DAYS_ABBREVIATIONS[] = {
    "ZO",
    "MA",
    "DI",
    "WO",
    "DO",
    "VR",
    "ZA",
};

#elif defined(LOCALE_IT_IT)

const char *I18N_MONTHS[] = {
    "GENNAIO",
    "FEBBRAIO",
    "MARZO",
    "APRILE",
    "MAGGIO",
    "GIUGNO",
    "LUGLIO",
    "AGOSTO",
    "SETTEMBRE",
    "OTTOBRE",
    "NOVEMBRE",
    "DICEMBRE",
};

const char *I18N_DAYS[] = {
    "DOMENICA",
    "LUNEDÌ",
    "MARTEDÌ",
    "MERCOLEDÌ",
    "GIOVEDÌ",
    "VENERDÌ",
    "SABATO",
};

const char *I18N_DAYS_ABBREVIATIONS[] = {
    "DOM",
    "LUN",
    "MAR",
    "MER",
    "GIO",
    "VEN",
    "SAB",
};

#elif defined(LOCALE_SV_SE)

const char *I18N_MONTHS[] = {
    "JANUARI",
    "FEBRUARI",
    "MARS",
    "APRIL",
    "MAJ",
    "JUNI",
    "JULI",
    "AUGUSTI",
    "SEPTEMBER",
    "OKTOBER",
    "NOVEMBER",
    "DECEMBER",
};

const char *I18N_DAYS[] = {
    "SÖNDAG",
    "MÅNDAG",
    "TISDAG",
    "ONSDAG",
    "TORSDAG",
    "FREDAG",
    "LÖRDAG",
};

const char *I18N_DAYS_ABBREVIATIONS[] = {
    "SÖN",
    "MÅN",
    "TIS",
    "ONS",
    "TORS",
    "FRE",
    "LÖR",
};

#else

#error No locale specified

#endif

#endif // PORTALCALENDAR_LOCALIZATION_H
