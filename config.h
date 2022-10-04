#define WIFI_NAME "name"
#define WIFI_PASS "password"

#define SHOW_DAY
#define SHOW_MONTH
// #define SHOW_YEAR

/**
 * If timezone should be looked up based on IP address using a 3rd-party service.
 */
#define AUTOMATIC_TIME_ZONE

/**
 * The Olson name for your timezone. Requires connection with a 3rd-party service to translate this to
 * the POSIX timezone specification. If AUTOMATIC_TIME_ZONE is set, this will only serve as a backup to that.
 */
#define MANUAL_TIME_ZONE "America/Chicago"

/**
 * The full POSIX specification for your timezone. If AUTOMATIC_TIME_ZONE or MANUAL_TIME_ZONE are set, this will
 * only serve as a backup to those. If not, then no 3rd-party timezone services will be used, however this will
 * need to be changed if your timezone or DST laws change in the future.
 */
#define POSIX_TIME_ZONE "CST6CDT,M3.2.0,M11.1.0"

/**
 * Show weather forecast in place of the chamber hazard icons
 */
#define SHOW_WEATHER

/**
 * 1: Show a 5-day weather forecast
 * 2: Show today's weather forecast in 3-hour intervals
 */
#define WEATHER_DISPLAY_TYPE 1

/**
 * If you choose to show today's weather in 3-hour intervals, this selects what other information is shown under
 * the temperature (in 5-day forecast mode, the high and low temperature are shown so this isn't an option).
 * 
 * 1: Chance of precipitation
 * 2: Humidity
 */
#define SECONDARY_WEATHER_INFORMATION 1

/**
 * If you choose to show today's weather in 3-hour intervals, this will display times in 24-hour format instead of 12-hour.
 */
// #define SHOW_24_HOUR_TIME

/**
 * metric or imperial
 */
#define WEATHER_UNITS "imperial"

/**
 * The location to get weather information for. Can also be a zip code for US locations.
 */
#define WEATHER_LOCATION u8"Oklahoma City, Oklahoma, US"

/**
 * The latitude and longitude for weather information. Optional. Overrides WEATHER_LOCATION if set.
 */
// #define WEATHER_LOCATION_LATITUDE 0.0
// #define WEATHER_LOCATION_LONGITUDE 0.0

/**
 * Show what the day's weather will be for the next 12 hours, starting at this hour (in 24-hour time).
 * 9 = 9am, 12 = 12pm, 15 = 3pm, etc
 */
#define WEATHER_START_HOUR 9

/**
 * Your API key for openweathermap.org
 * 
 * https://home.openweathermap.org/api_keys
 */
#define OPENWEATHERMAP_API_KEY ""

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
 * Two NTP syncs per day are performed because the internal clock in the ESP32 is very inaccurate.
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
 */
#define MINUTES_BEFORE_MIDNIGHT_TO_SYNC_1 72
#define MINUTES_BEFORE_MIDNIGHT_TO_SYNC_2 8

/**
 * How many hours we'll go without internet before showing an error. This really shouldn't be longer than a couple days, since the internal
 * clock in the ESP32 is usually off by double-digit minutes per day.
 */
#define ERROR_AFTER_HOURS_WITHOUT_INTERNET 24

/**
 * How long to wait before retrying after an error connecting to WiFi or syncing with NTP.
 */
#define ERROR_RETRY_INTERVAL_MINUTES 60

/**
 * Measure drift in the system clock and apply a correction factor for more accurate deep sleep.
 */
#define ENABLE_RTC_CORRECTION
#define MAX_RTC_CORRECTION_FACTOR 0.025

/**
 * Port assignments
 */
#define NTP_LOCAL_PORT_START 4242
#define TIMEZONED_LOCAL_PORT 2342

/**
 * Pin assignments
 */
#define SPI_BUS         HSPI
#define RESET_PIN       33
#define DC_PIN          23
#define CS_PIN          15
#define BUSY_PIN        27

/**
 * If debug logs should be printed over serial
 */
#define DEBUG
