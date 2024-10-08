#include "global.h"

#ifndef PORTALCALENDAR_TIME_H
#define PORTALCALENDAR_TIME_H

extern time_t lastNtpSync;

/**
 * The 150khz RC oscillator that the ESP32 uses in deep sleep is extremely inaccurate, but it's fairly
 * consistent if kept at the same temperature. This value is calculated on each NTP sync and is used
 * to adjust the time spent in deep sleep.
 */
extern float rtcCorrectionFactor;

/**
 * Caches the current POSIX timezone string
 */
extern char savedTimezone[];

/**
 * Adjusts the provided sleep duration by the RTC correction factor.
 */
void correctSleepDuration(time_t *timeAsleep);

/**
 * Updates the system clock to reflect the actual time after waking up from a sleep corrected by correctSleepDuration
 */
void correctSystemClock(time_t timeAsleep);

int getSecondsToMidnight(tm *now);

int getDaysInMonth(int month, int year);

void advanceDay(int& month, int& mday, int& year);

/**
 * Sets the system timezone from savedTimezone, and returns true if successful. If a timezone is provided,
 * the timezone will instead be set to that and it will be stored in savedTimezone.
 */
bool setTimezone(const char* tz = nullptr);

bool isSystemTimeValid();

enum class TimezonedResult : uint8_t {
    Ok,
    TzNotFound,
    ServerError,
};

/**
 * Looks up a timezone by Olson name using the timezoned.rop.nl service, which is part of the ezTime project (https://github.com/ropg/ezTime),
 * although I decided using the entirety of ezTime wasn't ideal.
 * 
 * This can also lookup timezone by IP address by passing "GeoIP", however I've found that pretty unreliable for where I live.
 */
TimezonedResult getPosixTz(std::initializer_list<const String> servers, const String name, String &result);

/**
 * Based on the queryNTP function from ezTime
 * https://github.com/ropg/ezTime
 *
 * @return True if the NTP sync was successful
 */
bool syncNtp(std::initializer_list<const String> servers, bool test = false);

#if CORE_DEBUG_LEVEL > 0
const char* printTime(time_t t);
#endif

#endif // PORTALCALENDAR_TIME_H
