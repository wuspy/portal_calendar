#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "time_util.h"

#define NTP_PACKET_SIZE 48

const char* NTP_SERVER_LIST[] = {NTP_SERVERS};
const char* TIMEZONED_SERVER_LIST[] = {TIMEZONED_SERVERS};

static_assert(
    sizeof(NTP_SERVER_LIST) / sizeof(char*) > 0,
    "No NTP servers configured"
);

static_assert(
    sizeof(TIMEZONED_SERVER_LIST) / sizeof(char*) > 0,
    "No timezoned servers configured"
);

RTC_DATA_ATTR time_t lastNtpSync = 0;

time_t getLastNtpSync()
{
    return lastNtpSync;
}

#ifdef ENABLE_RTC_CORRECTION
RTC_DATA_ATTR double rtcCorrectionFactor = 0.0;

double getRtcCorrectionFactor()
{
    return rtcCorrectionFactor;
}
#endif // ENABLE_RTC_CORRECTION

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

String getPosixTz(String name)
{
    for (int i = 0; i < sizeof(TIMEZONED_SERVER_LIST) / sizeof(char*); ++i) {
        const char* server = TIMEZONED_SERVER_LIST[i];
        DEBUG_PRINT("Looking up POSIX timezone for %s from %s", name.c_str(), server);
        WiFiUDP udp;
        udp.flush();
        udp.begin(TIMEZONED_LOCAL_PORT_START + i); // Each server must be called on a different port in case a packet comes in late
        unsigned long started = millis();
        udp.beginPacket(server, 2342);
        udp.write((const uint8_t*)name.c_str(), name.length());
        udp.endPacket();
        
        // Wait for packet until timeout
        bool parsedPacket;
        do {
            delay(1);
            parsedPacket = udp.parsePacket();
        } while (!parsedPacket && millis() - started < TZ_LOOKUP_TIMEOUT_MS);

        if (!parsedPacket) {
            DEBUG_PRINT("Timeout for server %s", server);
            udp.stop();
            continue;
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

        // Wait for packet until timeout
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
            .tv_sec = (time_t)(secsSince1900 - 2208988800UL) + us / uS_PER_S, // Subtract 70 years to get seconds since 1970
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
    return false;
}
