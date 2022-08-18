#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "config.h"
#include "Display.h"

#if !defined(AUTOMATIC_TIME_ZONE) && !defined(MANUAL_TIME_ZONE) && !defined(POSIX_TIME_ZONE)
    #error No timezone configured
#endif

#ifdef DEBUG
#define DEBUG_PRINT(...) Serial.printf(__VA_ARGS__); Serial.print('\n')
#else
#define DEBUG_PRINT(...)
#endif

#define uS_PER_S 1000000

#define ERROR_RETRY_INTERVAL_SECONDS            ERROR_RETRY_INTERVAL_MINUTES * 60
#define ERROR_AFTER_SECONDS_WITHOUT_INTERNET    ERROR_AFTER_HOURS_WITHOUT_INTERNET * 3600
#define NTP_TIMEOUT_MS                          NTP_TIMEOUT_SECONDS * 1000
#define SECONDS_BEFORE_MIDNIGHT_TO_SYNC_1       MINUTES_BEFORE_MIDNIGHT_TO_SYNC_1 * 60
#define SECONDS_BEFORE_MIDNIGHT_TO_SYNC_2       MINUTES_BEFORE_MIDNIGHT_TO_SYNC_2 * 60
#define TZ_LOOKUP_TIMEOUT_MS                    TZ_LOOKUP_TIMEOUT_SECONDS * 1000

#define NTP_PACKET_SIZE 48

#define clamp(x, y) min(max(x, -y), y)

const char* NTP_SERVER_LIST[] = {NTP_SERVERS};
const char* TIMEZONED_SERVER_LIST[] = {TIMEZONED_SERVERS};

enum TimezoneType: uint8_t
{
    TZT_NONE,
    TZT_POSIX,
    TZT_MANUAL,
    TZT_GEOIP,
};

/**
 * Stores the current day of the year displayed
 */
RTC_DATA_ATTR int displayedYDay = 0;

/**
 * Controls if initTime() will try to sync with NTP when called.
 *
 * Can be set to true before going to sleep if a sync is desired on the next wakeup, however
 * initTime() is the only place this should ever be changed to false.
 */
RTC_DATA_ATTR bool needsNtpSync = true;
RTC_DATA_ATTR time_t lastNtpSync = 0;

#ifdef ENABLE_RTC_CORRECTION
/**
 * The 150khz RC oscillator that the ESP32 uses in deep sleep is extremely inaccurate, but it's fairly
 * consistent if kept at the same temperature. This value is calculated on each NTP sync and is used
 * to adjust the time spent in deep sleep.
 */
RTC_DATA_ATTR double rtcCorrectionFactor = 0.0;
#endif

/**
 * Controls if initTime() will try to sync timezone information when called.
 *
 * Can be set to true before going to sleep if a sync is desired on the next wakeup, however
 * initTime() is the only place this should ever be changed to false.
 */
RTC_DATA_ATTR bool needsTimezoneSync = true;
RTC_DATA_ATTR char savedTimezone[57];
RTC_DATA_ATTR TimezoneType savedTimezoneType = TZT_NONE;

Display display;

void deepSleep(uint64_t seconds)
{
    DEBUG_PRINT("Sleeping for %lus", seconds);
    unsigned long start = millis(); // Stopping wifi can take time
    stopWifi();
    #ifdef ENABLE_RTC_CORRECTION
    if (rtcCorrectionFactor) {
        // Adjust the duration to sleep by the correction factor, and adjust the system time so
        // we still wake up at the time we think we will
        uint64_t adjustment = (uint64_t)round((double)seconds * rtcCorrectionFactor);
        if (adjustment) {
            timeval now;
            gettimeofday(&now, nullptr);
            now.tv_sec -= (time_t)adjustment;
            settimeofday(&now, nullptr);
            seconds += adjustment;
            DEBUG_PRINT("Sleeping for %lus (corrected)", seconds);
        }
    }
    #endif
    uint64_t duration = millis() - start;
    esp_sleep_enable_timer_wakeup(seconds * uS_PER_S - duration * 1000);
    esp_deep_sleep_start();
}

void stopWifi()
{
    if (WiFi.getMode() != WIFI_OFF) {
        DEBUG_PRINT("Stopping WiFi");
        unsigned long start = millis();
        WiFi.disconnect(true, true);
        WiFi.mode(WIFI_OFF);
        DEBUG_PRINT("WiFi shutdown took %lums", millis() - start);
    }
}

bool startWifi()
{
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }
    DEBUG_PRINT("Starting WiFi");
    unsigned long start = millis();
    WiFi.begin(WIFI_NAME, WIFI_PASS);
    if (WiFi.waitForConnectResult(10000) != WL_CONNECTED) {
        DEBUG_PRINT("WiFi connection failed after %lums", millis() - start);
        stopWifi();
        return false;
    }
    DEBUG_PRINT("WiFi connection took %lums", millis() - start);
    return true;
}

void deepSleepWithError(const char *message)
{
    DEBUG_PRINT("Sleeping with error '%s'", message);
    stopWifi(); // Power down wifi before updating display to limit current draw from battery
    display.error(message);
    deepSleep(ERROR_RETRY_INTERVAL_SECONDS);
}

#if defined(AUTOMATIC_TIME_ZONE) || defined(MANUAL_TIME_ZONE)

/**
 * Looks up a timezone by Olson name using the timezoned.rop.nl service, which is part of the ezTime project (https://github.com/ropg/ezTime),
 * although I decided using the entirety of ezTime wasn't ideal.
 * 
 * This can also lookup timezone by IP address by passing "GeoIP", however I've found that pretty unreliable for where I live.
 * 
 * @return The POSIX timezone string, or empty string if unsuccessful
 */
String getPosixTz(String olsonOrGeoIp)
{
    for (int i = 0; i < sizeof(TIMEZONED_SERVER_LIST) / sizeof(char*); ++i) {
        const char* server = TIMEZONED_SERVER_LIST[i];
        DEBUG_PRINT("Looking up POSIX timezone for %s from %s", olsonOrGeoIp.c_str(), server);
        WiFiUDP udp;
        udp.flush();
        udp.begin(TIMEZONED_LOCAL_PORT);
        unsigned long started = millis();
        udp.beginPacket(server, 2342);
        udp.write((const uint8_t*)olsonOrGeoIp.c_str(), olsonOrGeoIp.length());
        udp.endPacket();
        
        // Wait for packet or return false with timed out
        while (!udp.parsePacket()) {
            delay (1);
            if (millis() - started > TZ_LOOKUP_TIMEOUT_MS) {
                udp.stop();
                DEBUG_PRINT("Timeout for %s", server);
                return "";
            }
        }
        DEBUG_PRINT("Request to %s request took %lums", server, millis() - started);
        // Stick result in String recv 
        String recv;
        recv.reserve(60);
        while (udp.available()) {
            recv += (char)udp.read();
        }
        udp.stop();
        DEBUG_PRINT("Response from %s: %s", server, recv.c_str());
        if (recv.substring(0, 3) == "OK ") {
            return recv.substring(recv.indexOf(" ", 4) + 1);
        }
    }
    return "";
}

#endif

#if defined(AUTOMATIC_TIME_ZONE)

/**
 * Looks up a timezone by IP address using worldtimeapi.org and timezoned.rop.nl.
 * Currently, worldtimeapi.org doesn't return a POSIX timezone string, just the Olson name, so even
 * if it's successful a call to timezoned.rop.nl is still required.
 * 
 * @return The POSIX timezone string, or empty string if unsuccessful 
 */
String getGeoIpTz()
{
    DEBUG_PRINT("Looking up GeoIP timezone");
    String olsonOrGeoIp = "GeoIP";
    HTTPClient http;
    unsigned long start = millis();
    http.setConnectTimeout(TZ_LOOKUP_TIMEOUT_MS);
    http.begin("http://worldtimeapi.org/api/ip.txt");
    if (http.GET() == 200) {
        String response = http.getString();
        DEBUG_PRINT("Request to worldtimeapi took %lums", millis() - start);
        int startIdx = response.indexOf("timezone: ");
        if (startIdx != -1) {
            int endIdx = response.indexOf('\n', startIdx);
            if (endIdx != -1) {
                olsonOrGeoIp = response.substring(startIdx + 10, endIdx);
                DEBUG_PRINT("Got timezone %s for GeoIP", olsonOrGeoIp.c_str());
            }
        }
    } else {
        DEBUG_PRINT("Request to worldtimeapi failed after %lums", millis() - start);
    }
    http.end();
    return getPosixTz(olsonOrGeoIp);
}

#endif

/**
 * Based on the queryNTP function from ezTime
 * https://github.com/ropg/ezTime
 *
 * @return True if the NTP sync was successful
 */
bool syncNtp()
{
    for (int i = 0; i < sizeof(NTP_SERVER_LIST) / sizeof(char*); ++i) {
        const char *server = NTP_SERVER_LIST[i];
        DEBUG_PRINT("Starting NTP request to %s", server);

        // Send NTP packet
        byte buffer[NTP_PACKET_SIZE];
        memset(buffer, 0, NTP_PACKET_SIZE);
        buffer[0] = 0b11100011;     // LI, Version, Mode
        buffer[1] = 0;              // Stratum, or type of clock
        buffer[2] = 9;              // Polling Interval (9 = 2^9 secs = ~9 mins, close to our 10 min default)
        buffer[3] = 0xEC;           // Peer Clock Precision
                                    // 8 bytes of zero for Root Delay & Root Dispersion
        buffer[12]  = 'X';          // "kiss code", see RFC5905
        buffer[13]  = 'E';          // (codes starting with 'X' are not interpreted)
        buffer[14]  = 'Z';
        buffer[15]  = 'T';	

        WiFiUDP udp;
        udp.flush();
        udp.begin(NTP_LOCAL_PORT_START + i); // Each server must be called on a different port in case a packet comes in late
        unsigned long started = millis();
        udp.beginPacket(server, 123); //NTP requests are to port 123
        udp.write(buffer, NTP_PACKET_SIZE);
        udp.endPacket();

        // Wait for packet or return false with timed out
        bool parsedPacket;
        do {
            delay(1);
            parsedPacket = udp.parsePacket();
        } while (!parsedPacket && millis() - started < NTP_TIMEOUT_MS);
    
        if (!parsedPacket) {
            DEBUG_PRINT("NTP sync timeout for server %s", server);
            udp.stop();
            continue;
        }
        udp.read(buffer, NTP_PACKET_SIZE);
        udp.stop();

        //prepare timestamps
        uint32_t highWord, lowWord;	
        highWord = ( buffer[16] << 8 | buffer[17] ) & 0x0000FFFF;
        lowWord = ( buffer[18] << 8 | buffer[19] ) & 0x0000FFFF;
        uint32_t reftsSec = highWord << 16 | lowWord;				// reference timestamp seconds

        highWord = ( buffer[32] << 8 | buffer[33] ) & 0x0000FFFF;
        lowWord = ( buffer[34] << 8 | buffer[35] ) & 0x0000FFFF;
        uint32_t rcvtsSec = highWord << 16 | lowWord;				// receive timestamp seconds

        highWord = ( buffer[40] << 8 | buffer[41] ) & 0x0000FFFF;
        lowWord = ( buffer[42] << 8 | buffer[43] ) & 0x0000FFFF;
        uint32_t secsSince1900 = highWord << 16 | lowWord;			// transmit timestamp seconds

        highWord = ( buffer[44] << 8 | buffer[45] ) & 0x0000FFFF;
        lowWord = ( buffer[46] << 8 | buffer[47] ) & 0x0000FFFF;
        uint32_t fraction = highWord << 16 | lowWord;				// transmit timestamp fractions	

        //check if received data makes sense
        //buffer[1] = stratum - should be 1..15 for valid reply
        //also checking that all timestamps are non-zero and receive timestamp seconds are <= transmit timestamp seconds
        if ((buffer[1] < 1) or (buffer[1] > 15) or (reftsSec == 0) or (rcvtsSec == 0) or (rcvtsSec > secsSince1900)) {
            // we got invalid packet
            DEBUG_PRINT("NTP sync failed for server %s", server);
            continue;
        }

        // Set the t and measured_at variables that were passed by reference
        unsigned long duration = millis() - started;
        DEBUG_PRINT("NTP sync took %lums", duration);
        suseconds_t us = (fraction / 4294967UL + duration / 2) * 1000; // Assume symmetric network latency
        timeval now = {
            .tv_sec = secsSince1900 - 2208988800UL + us / uS_PER_S, // Subtract 70 years to get seconds since 1970
            .tv_usec = us % uS_PER_S,
        };
        timeval oldNow;
        gettimeofday(&oldNow, nullptr);
        settimeofday(&now, nullptr);
        if (lastNtpSync && difftime(now.tv_sec, lastNtpSync) > 30) {
            long driftMs = (now.tv_sec - oldNow.tv_sec) * 1000 + (now.tv_usec - oldNow.tv_usec) / 1000;
            #ifdef ENABLE_RTC_CORRECTION
            rtcCorrectionFactor -= (double)driftMs / 1000.0 / difftime(now.tv_sec, lastNtpSync);
            rtcCorrectionFactor = clamp(rtcCorrectionFactor, MAX_RTC_CORRECTION_FACTOR);
            DEBUG_PRINT("System clock drift was %ldms, new correction factor is %0.4f", driftMs, rtcCorrectionFactor);
            #else
            DEBUG_PRINT("System clock drift was %ldms", driftMs);
            #endif
        }
        lastNtpSync = now.tv_sec;
        return true;
    }
}

/**
 * Initializes the system time and timezone, syncing with NTP if necessary.
 *
 * If NTP sync fails and the system time has never been set before, this function will sleep and won't return.
 */
void initTime()
{
    if (needsTimezoneSync || savedTimezoneType == TZT_NONE) {
        if (startWifi()) {
            #if defined(AUTOMATIC_TIME_ZONE) || defined(MANUAL_TIME_ZONE)
            String tz;
            #endif
            #ifdef AUTOMATIC_TIME_ZONE
            tz = getGeoIpTz();
            if (!tz.isEmpty()) {
                strcpy(savedTimezone, tz.c_str());
                savedTimezoneType = TZT_GEOIP;
                needsTimezoneSync = false;
            }
            #endif
            #ifdef MANUAL_TIME_ZONE
            if (savedTimezoneType <= TZT_MANUAL) {
                tz = getPosixTz(MANUAL_TIME_ZONE);
                if (!tz.isEmpty()) {
                    strcpy(savedTimezone, tz.c_str());
                    savedTimezoneType = TZT_MANUAL;
                    needsTimezoneSync = false;
                }
            }
            #endif
            #ifdef POSIX_TIME_ZONE
            if (savedTimezoneType <= TZT_POSIX) {
                strcpy(savedTimezone, POSIX_TIME_ZONE);
                savedTimezoneType = TZT_POSIX;
                needsTimezoneSync = false;
            }
            #endif

            if (savedTimezoneType == TZT_NONE) {
                deepSleepWithError("Timezone lookup failed");
            }
        } else if (savedTimezoneType == TZT_NONE) {
            // WiFi didn't connect and we have no idea what timezone we're in
            deepSleepWithError("WiFi connection failed");
        }
    }

    // Set timezone
    setenv("TZ", savedTimezone, 1);
    tzset();

    if (needsNtpSync || !lastNtpSync) {
        bool syncMandatory = !lastNtpSync || (time_t)difftime(time(nullptr), lastNtpSync) >= ERROR_AFTER_SECONDS_WITHOUT_INTERNET;
        if (startWifi()) {
            if (syncNtp()) {
                needsNtpSync = false;
            } else if (syncMandatory) {
                // Sync unsuccesful and we have no idea what time it is
                deepSleepWithError("Time syncronization failed");
            }
        } else if (syncMandatory) {
            // WiFi didn't connect and we have no idea what time it is
            deepSleepWithError("WiFi connection failed");
        }
    }
}

int getSecondsToMidnight(tm *now)
{
    tm tomorrow = *now;
    ++tomorrow.tm_mday; // mktime will handle day/month rolling over
    tomorrow.tm_hour = tomorrow.tm_min = tomorrow.tm_sec = 0;
    return (int)difftime(mktime(&tomorrow), mktime(now));
}

void setup()
{
    time_t t;
    tm now;
    
    #ifdef DEBUG
    Serial.begin(115200);
    time(&t);
    localtime_r(&t, &now);
    char timestr[30];
    strftime(timestr, sizeof(timestr), "%d-%m-%Y %H:%M:%S", &now);
    DEBUG_PRINT("Waking up at %s", timestr);
    #endif

    // Figure out what time it is

    initTime();
    time(&t);
    localtime_r(&t, &now);

    stopWifi(); // Power down wifi before updating display to limit current draw from battery

    // Update display if needed
    
    if (now.tm_yday != displayedYDay) {
        DEBUG_PRINT("Updating display for %d-%d-%d", now.tm_year + 1900, now.tm_mon + 1, now.tm_mday);
        display.update(&now);
        displayedYDay = now.tm_yday;
    }

    // Go to sleep

    time(&t); // Update time measurement
    localtime_r(&t, &now);
    int secondsToMidnight = getSecondsToMidnight(&now) + 1; // +1 to make sure it's actually at or past midnight
    if (needsNtpSync) {
        // Last NTP sync failed
        DEBUG_PRINT("NTP sync failed, will retry");
        if (secondsToMidnight <= SECONDS_BEFORE_MIDNIGHT_TO_SYNC_2) {
            deepSleep(min(secondsToMidnight, SECONDS_BEFORE_MIDNIGHT_TO_SYNC_2 / 3));
        } else if (secondsToMidnight <= SECONDS_BEFORE_MIDNIGHT_TO_SYNC_1) {
            deepSleep(min(secondsToMidnight - SECONDS_BEFORE_MIDNIGHT_TO_SYNC_2, (SECONDS_BEFORE_MIDNIGHT_TO_SYNC_1 - SECONDS_BEFORE_MIDNIGHT_TO_SYNC_2) / 3));
        } else {
            deepSleep(min(secondsToMidnight - SECONDS_BEFORE_MIDNIGHT_TO_SYNC_1, ERROR_AFTER_SECONDS_WITHOUT_INTERNET / 5));
        }
    } else if (secondsToMidnight <= SECONDS_BEFORE_MIDNIGHT_TO_SYNC_2 * 2) {
        // Sleep until midnight
        DEBUG_PRINT("Sleeping until midnight");
        deepSleep(secondsToMidnight);
    } else if (secondsToMidnight <= SECONDS_BEFORE_MIDNIGHT_TO_SYNC_1 * 2) {
        // Sleep until second NTP sync
        needsNtpSync = true;
        DEBUG_PRINT("Sleeping for 2nd NTP sync");
        deepSleep(secondsToMidnight - SECONDS_BEFORE_MIDNIGHT_TO_SYNC_2);
    } else {
        // Sleep until first NTP sync
        needsNtpSync = true;
        needsTimezoneSync = true;
        DEBUG_PRINT("Sleeping for 1st NTP sync");
        deepSleep(secondsToMidnight - SECONDS_BEFORE_MIDNIGHT_TO_SYNC_1);
    }
}

void loop()
{
}
