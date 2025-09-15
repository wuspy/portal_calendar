#include "global.h"

#ifndef PORTALCALENDAR_LOCALIZATION_H
#define PORTALCALENDAR_LOCALIZATION_H

struct Locale
{
    const char *code;
    const char *name;
    const char *months[12];
    const char *days[7];
    const char *dayAbbreviations[7];
};

const Locale LOCALES[] = {
    {
        .code = "de",
        .name = "Deutsch",
        .months = {
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
        },
        .days = {"SONNTAG", "MONTAG", "DIENSTAG", "MITTWOCH", "DONNERSTAG", "FREITAG", "SAMSTAG"},
        .dayAbbreviations = {"SO", "MO", "DI", "MI", "DO", "FR", "SA"},
    },
    {
        .code = "en",
        .name = "English",
        .months = {
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
        },
        .days = {"SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY"},
        .dayAbbreviations = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"},
    },
    {
        .code = "es",
        .name = "Español",
        .months = {
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
        },
        .days = {"DOMINGO", "LUNES", "MARTES", "MIÉRCOLES", "JUEVES", "VIERNES", "SÁBADO"},
        .dayAbbreviations = {"DOM", "LUN", "MAR", "MIÉ", "JUE", "VIE", "SÁB"},
    },
    {
        .code = "fr",
        .name = "Français",
        .months = {
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
        },
        .days = {"DIMANCHE", "LUNDI", "MARDI", "MERCREDI", "JEUDI", "VENDREDI", "SAMEDI"},
        .dayAbbreviations = {"DIM.", "LUN.", "MAR.", "MER.", "JEU.", "VEN.", "SAM."},
    },
    {
        .code = "it",
        .name = "Italiano",
        .months = {
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
        },
        .days = {"DOMENICA", "LUNEDÌ", "MARTEDÌ", "MERCOLEDÌ", "GIOVEDÌ", "VENERDÌ", "SABATO"},
        .dayAbbreviations = {"DOM", "LUN", "MAR", "MER", "GIO", "VEN", "SAB"},
    },
    {
        .code = "nl",
        .name = "Nederlands",
        .months = {
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
        },
        .days = {"ZONDAG", "MAANDAG", "DINSDAG", "WOENSDAG", "DONDERDAG", "VRIJDAG", "ZATERDAG"},
        .dayAbbreviations = {"ZO", "MA", "DI", "WO", "DO", "VR", "ZA"},
    },
    {
        .code = "sv",
        .name = "Svenska",
        .months = {
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
        },
        .days = {"SÖNDAG", "MÅNDAG", "TISDAG", "ONSDAG", "TORSDAG", "FREDAG", "LÖRDAG"},
        .dayAbbreviations = {"SÖN", "MÅN", "TIS", "ONS", "TORS", "FRE", "LÖR"},
    },
    {
        .code = "pt",
        .name = "Português",
        .months = {
          "JANEIRO",
          "FEVEREIRO",
          "MARÇO",
          "ABRIL",
          "MAIO",
          "JUNHO",
          "JULHO",
          "AGOSTO",
          "SETEMBRO",
          "OUTUBRO",
          "NOVEMBRO",
          "DEZEMBRO",
        },
        .days = {"DOMINGO", "SEGUNDA", "TERÇA", "QUARTA", "QUINTA", "SEXTA", "SÁBADO"},
        .dayAbbreviations = {"DOM", "SEG", "TER", "QUA", "QUI", "SEX", "SAB"},
    },
};

const Locale getLocale(String code);

#endif // PORTALCALENDAR_LOCALIZATION_H
