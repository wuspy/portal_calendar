#include "localization.h"

const Locale getLocale(String code)
{
    for (Locale locale: LOCALES) {
        if (code.equals(locale.code)) {
            return locale;
        }
    }
    if (code.equals(DEFAULT_LOCALE)) {
        DEBUG_PRINT("Default locale '" DEFAULT_LOCALE "' not found!");
        return LOCALES[0];
    }
    DEBUG_PRINT("Locale '%s' not found, returning default locale '" DEFAULT_LOCALE "'", code.c_str());
    return getLocale(DEFAULT_LOCALE);
}
