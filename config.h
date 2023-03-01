// Name of the temporary Access Point you connect your phone to to configure the device
#define WIFI_AP_NAME "Aperture Science Guest"

// Must be at least 8 chars or AP setup will fail.
#define WIFI_AP_PASSWORD "Aperture"

// Version 5 gives us a maximum of 154 chars with lowest ECC (needed to encode both WIFI_AP_NAME + WIFI_AP_PASSWORD + a few formatting chars)
// Too large of a value causes a stack overflow when generating QR code (will look like device boot loop/frozen).
 #define MAX_QRCODE_VERSION 5

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
//   You probably don't need to edit anything below here   //
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

#define NTP_SERVERS "pool.ntp.org", "time.google.com"
#define TIMEZONED_SERVERS "timezoned.rop.nl"

/**
 * How long we'll wait for an NTP sync before giving up.
 * This is PER SERVER, so if there's no internet connection and 3 servers, the total timeout will be 3x this amount.
 */
#define NTP_TIMEOUT_SECONDS 5

/**
 * How long we'll wait for a timezone information lookup before giving up.
 * This is PER SERVER, so if there's no internet connection and 3 servers, the total timeout will be 3x this amount.
 */
#define TZ_LOOKUP_TIMEOUT_SECONDS 5

/**
 * Controls how long before midnight the processor is woken up for the first and second NTP syncs.
 * 
 * Two NTP syncs per day are performed per day because the internal clock in the ESP32 is very inaccurate.
 * 
 * The first time should be set to the maximum possible amount you expect the internal clock to be off in one day, since it will sleep for an entire day
 * and be woken at this time. The second one should be set to the maximum possible amount it will drift in the time betwen the first sync and midnight.
 * 
 * If the clock is running +MINUTES_BEFORE_MIDNIGHT_TO_SYNC_1 fast per day, then in reality it will be woken up MINUTES_BEFORE_MIDNIGHT_TO_SYNC_1 * 2
 * minutes before midnight, which is the maximum duration the NTP measurement will be deemed acceptable for. Any faster than that, and multiple
 * first stage NTP syncs will happen per day, which wastes battery.
 * 
 * If the clock is running -MINUTES_BEFORE_MIDNIGHT_TO_SYNC_1 slow per day, then it will wake up and sync exactly at midnight. Any slower than that,
 * and it won't wake wake up in time for midnight and the date changeover will be late.
 * 
 * The default values correspond to the maximum inaccuracy specified in the ESP32's datasheet, I recommend you don't touch them.
 */
#define MINUTES_BEFORE_MIDNIGHT_TO_SYNC_1 72
#define MINUTES_BEFORE_MIDNIGHT_TO_SYNC_2 8

/**
 * How many hours we'll go without internet before showing an error. This really shouldn't be longer than a couple days, since the internal
 * clock in the ESP32 is usually off by double-digit minutes per day.
 */
#define ERROR_AFTER_HOURS_WITHOUT_INTERNET 24

/**
 * Measure drift in the system clock and apply a correction factor for more accurate timekeeping.
 * 
 * Since the ESP32's internal clock drifts significantly based on temperature, this can be pretty effective at improving the clock's accuracy
 * assuming the clock is placed in a relatively temperature-stable environment. Like, you know, indoors.
 */
#define ENABLE_RTC_CORRECTION
#define MAX_RTC_CORRECTION_FACTOR 0.025

/**
 * Port assignments
 */
#define NTP_LOCAL_PORT_START 4242
#define TIMEZONED_LOCAL_PORT_START 2342

/**
 * Pin assignments
 */
#define SPI_BUS         HSPI
#define DIN_PIN         -1 // COPI
#define CLK_PIN         -1 // SCK
#define CS_PIN          15 // CS
#define DC_PIN          23 // Any OUTPUT pin
#define RESET_PIN       33 // Any OUTPUT pin
#define BUSY_PIN        27 // Any INPUT pin

/**
 * If debug logs should be printed over serial
 */
#define DEBUG
