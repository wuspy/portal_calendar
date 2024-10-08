#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <math.h>
#include <time.h>
#include "Configuration.h"
#include "time_util.h"

#define NTP_PACKET_SIZE 48
#define MIN_CORRECTABLE_SLEEP_DURATION 30

RTC_DATA_ATTR time_t lastNtpSync = 0;
RTC_DATA_ATTR float rtcCorrectionFactor = 0.0;
RTC_DATA_ATTR char savedTimezone[57] = {'\0'};

void correctSleepDuration(time_t *timeAsleep)
{
    if (*timeAsleep < MIN_CORRECTABLE_SLEEP_DURATION) {
        return;
    }

    time_t adjustment = (time_t)round((float)*timeAsleep * rtcCorrectionFactor);
    log_i("RTC correction for this sleep duration is %llds (factor %0.6f)", (long long)adjustment, rtcCorrectionFactor);
    *timeAsleep += adjustment;
}

void correctSystemClock(time_t timeAsleep)
{
    if (timeAsleep < MIN_CORRECTABLE_SLEEP_DURATION) {
        return;
    }

    float adjustment = (float)timeAsleep * rtcCorrectionFactor;
    log_i("Was asleep for %llds, adjusting system clock by %0.6fs (factor %0.6f)", (long long)timeAsleep, -adjustment, rtcCorrectionFactor);
    suseconds_t adjtv_usec = (suseconds_t)(std::modf(adjustment, &adjustment) * uS_PER_S);
    time_t adjtv_sec = (time_t)adjustment;
    timeval tvnow;
    gettimeofday(&tvnow, nullptr);
    tvnow.tv_sec -= adjtv_sec;
    tvnow.tv_usec -= (suseconds_t)adjtv_usec;
    if (tvnow.tv_usec >= uS_PER_S) {
        ++tvnow.tv_sec;
        tvnow.tv_usec %= uS_PER_S;
    } else if (tvnow.tv_usec < 0) {
        --tvnow.tv_sec;
        tvnow.tv_usec += uS_PER_S;
    }
    settimeofday(&tvnow, nullptr);
}

int getSecondsToMidnight(tm *now)
{
    tm tomorrow = *now;
    ++tomorrow.tm_mday; // mktime will handle day/month rolling over
    tomorrow.tm_hour = tomorrow.tm_min = tomorrow.tm_sec = 0;
    return static_cast<int>(ceil(difftime(mktime(&tomorrow), mktime(now))));
}

bool setTimezone(const char* tz)
{
    if (tz) {
        strcpy(savedTimezone, tz);
    }
    if (savedTimezone[0]) {
        log_i("Setting system timezone to %s", savedTimezone);
        setenv("TZ", savedTimezone, 1);
        tzset();
        return true;
    } else {
        log_i("Timezone is not configured");
        return false;
    }
}

bool isSystemTimeValid()
{
    return getenv("TZ")
        && lastNtpSync
        && static_cast<time_t>(ceil(difftime(time(nullptr), lastNtpSync))) < ERROR_AFTER_SECONDS_WITHOUT_INTERNET;
}

int getDaysInMonth(int month, int year)
{
    switch (month) {
        case 1:
            return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0) ? 29 : 28;
        case 3:
        case 5:
        case 8:
        case 10:
            return 30;
        default:
            return 31;
    }
}

void advanceDay(int& month, int& mday, int& year)
{
    ++mday;
    if (mday > getDaysInMonth(month, year)) {
        mday = 1;
        ++month;
        if (month > 11) {
            month = 0;
            ++year;
        }
    }
}

TimezonedResult getPosixTz(std::initializer_list<const String> servers, const String name, String &result)
{
    uint16_t i = TIMEZONED_LOCAL_PORT_START;
    for (String server : servers) {
        if (server.isEmpty()) {
            continue;
        }
        log_i("Looking up POSIX timezone for %s from %s", name.c_str(), server.c_str());

        WiFiUDP udp;
        udp.flush();
        if (!udp.begin(TIMEZONED_LOCAL_PORT_START + i++) // Each server must be called on a different port in case a packet comes in late
            || !udp.beginPacket(server.c_str(), 2342)
        ) {
            udp.stop();
            continue;
        }
        unsigned long started = millis();
        udp.write((const uint8_t*)name.c_str(), name.length());
        if (!udp.endPacket()) {
            udp.stop();
            continue;
        }

        // Wait for packet until timeout
        bool parsedPacket;
        do {
            yield();
            parsedPacket = udp.parsePacket();
        } while (!parsedPacket && millis() - started < TZ_LOOKUP_TIMEOUT_MS);

        if (!parsedPacket) {
            log_e("Timeout for server %s", server.c_str());
            udp.stop();
            continue;
        }

        log_i("Request to %s took %lums", server.c_str(), millis() - started);
        // Stick result in String recv
        String recv;
        recv.reserve(60);
        while (udp.available()) {
            recv += (char)udp.read();
        }
        udp.stop();
        log_i("Response from %s: %s", server.c_str(), result.c_str());
        if (recv.startsWith("OK ")) {
            result = recv.substring(recv.indexOf(" ", 4) + 1);
            return TimezonedResult::Ok;
        } else if (result.startsWith("ERR ")) {
            return TimezonedResult::TzNotFound;
        }
    }
    return TimezonedResult::ServerError;
}

/**
 * Based on the queryNTP function from ezTime
 * https://github.com/ropg/ezTime
 *
 * @return True if the NTP sync was successful
 */
bool syncNtp(std::initializer_list<const String> servers, bool test)
{
    uint16_t i = NTP_LOCAL_PORT_START;
    for (String server : servers) {
        if (server.isEmpty()) {
            continue;
        }
        log_i("Starting NTP request to %s", server.c_str());

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
        if (!udp.begin(NTP_LOCAL_PORT_START + i++) // Each server must be called on a different port in case a packet comes in late
            || !udp.beginPacket(server.c_str(), 123)
        ) {
            udp.stop();
            continue;
        }
        unsigned long started = millis();
        udp.write(buffer, NTP_PACKET_SIZE);
        if (!udp.endPacket()) {
            udp.stop();
            continue;
        }

        // Wait for packet until timeout
        int parsedPacket;
        do {
            yield();
            parsedPacket = udp.parsePacket();
        } while (!parsedPacket && millis() - started < NTP_TIMEOUT_MS);

        if (!parsedPacket) {
            log_e("NTP sync timeout for server %s", server.c_str());
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
            log_e("NTP sync failed for server %s", server.c_str());
            continue;
        }

        // Set the t and measured_at variables that were passed by reference
        unsigned long duration = millis() - started;
        log_i("NTP sync took %lums", duration);
        if (test) {
            return true;
        }
        suseconds_t us = (fraction / 4294967UL + duration / 2) * 1000; // Assume symmetric network latency
        timeval now = {
            .tv_sec = (time_t)(secsSince1900 - 2208988800UL) + us / uS_PER_S, // Subtract 70 years to get seconds since 1970
            .tv_usec = us % uS_PER_S,
        };
        timeval oldNow;
        gettimeofday(&oldNow, nullptr);
        settimeofday(&now, nullptr);
        log_i("Time after NTP sync is %s", printTime(now.tv_sec));
        float maxRtcCorrectionFactor = Config.getMaxRtcCorrectionFactor();
        long driftMs = (now.tv_sec - oldNow.tv_sec) * 1000 + (now.tv_usec - oldNow.tv_usec) / 1000;
        if (!maxRtcCorrectionFactor) {
            rtcCorrectionFactor = 0;
            log_i("System clock drift was %ldms", driftMs);
        } else if (lastNtpSync && difftime(now.tv_sec, lastNtpSync) > 30) {
            rtcCorrectionFactor -= (float)driftMs / 1000.0 / difftime(now.tv_sec, lastNtpSync);
            rtcCorrectionFactor = clamp(rtcCorrectionFactor, maxRtcCorrectionFactor);
            log_i("System clock drift was %ldms, new correction factor is %0.4f", driftMs, rtcCorrectionFactor);
        }
        lastNtpSync = now.tv_sec;
        return true;
    }
    return false;
}

#if CORE_DEBUG_LEVEL > 0
char timeStr[30];

const char* printTime(time_t t)
{
    tm now;
    localtime_r(&t, &now);
    strftime(timeStr, sizeof(timeStr), "%d-%m-%Y %H:%M:%S", &now);
    return timeStr;
}
#endif // CORE_DEBUG_LEVEL
