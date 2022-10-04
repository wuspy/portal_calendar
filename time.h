#include "global.h"

#if !defined(AUTOMATIC_TIME_ZONE) && !defined(MANUAL_TIME_ZONE) && !defined(POSIX_TIME_ZONE)
#error No timezone configured
#endif

#ifndef PORTALCALENDAR_TIME_H
#define PORTALCALENDAR_TIME_H

time_t getLastNtpSync();

/**
 * The 150khz RC oscillator that the ESP32 uses in deep sleep is extremely inaccurate, but it's fairly
 * consistent if kept at the same temperature. This value is calculated on each NTP sync and is used
 * to adjust the time spent in deep sleep.
 */
double getRtcCorrectionFactor();

int getDaysInMonth(int month, int year);

void advanceDay(int& month, int& mday, int& year);

#if defined(AUTOMATIC_TIME_ZONE) || defined(MANUAL_TIME_ZONE)

/**
 * Looks up a timezone by Olson name using the timezoned.rop.nl service, which is part of the ezTime project (https://github.com/ropg/ezTime),
 * although I decided using the entirety of ezTime wasn't ideal.
 * 
 * This can also lookup timezone by IP address by passing "GeoIP", however I've found that pretty unreliable for where I live.
 * 
 * @return The POSIX timezone string, or empty string if unsuccessful
 */
String getPosixTz(String olsonOrGeoIp);

#endif // defined(AUTOMATIC_TIME_ZONE) || defined(MANUAL_TIME_ZONE)

#if defined(AUTOMATIC_TIME_ZONE)

/**
 * Looks up a timezone by IP address using worldtimeapi.org and timezoned.rop.nl.
 * Currently, worldtimeapi.org doesn't return a POSIX timezone string, just the Olson name, so even
 * if it's successful a call to timezoned.rop.nl is still required.
 * 
 * @return The POSIX timezone string, or empty string if unsuccessful 
 */
String getGeoIpTz();

#endif // defined(AUTOMATIC_TIME_ZONE)

/**
 * Based on the queryNTP function from ezTime
 * https://github.com/ropg/ezTime
 *
 * @return True if the NTP sync was successful
 */
bool syncNtp();

#endif // PORTALCALENDAR_TIME_H
