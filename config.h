/**
 * Enter the WiFi network you want to connect to, for getting the current time and other information.
 * If for some reason your WiFi doesn't have a password, you can comment that out.
 */
#define WIFI_NAME "your wifi name here"
#define WIFI_PASS "your wifi password here"
#define HOSTNAME "portal_calendar"

/**
 * Show the day name on the right side (next to the XX/XX day)
 */
#define SHOW_DAY

/**
 * Show the month name on the top
 */
#define SHOW_MONTH

/**
 * Show the current year on the top right
 */
// #define SHOW_YEAR

/**
 * The name of your timezone. Requires connection with a 3rd-party service to get all the DST & offset information.
 * 
 * You can find a list of timezone names here:
 * https://en.wikipedia.org/wiki/List_of_tz_database_time_zones
 */
#define TIME_ZONE "America/Chicago"

/**
 * The full POSIX specification for your timezone. If TIME_ZONE is set, this will only serve as a backup to that.
 * If not, then no 3rd-party timezone services will be used, however this will need to be changed if your timezone
 * or DST laws change in the future.
 */
// #define POSIX_TIME_ZONE "CST6CDT,M3.2.0,M11.1.0"

/**
 * Show weather forecast in place of the chamber icons
 */
// #define SHOW_WEATHER

/**
 * Your API key for openweathermap.org, which is the service used to get the weather for your location.
 * This is REQUIRED to use the weather feature. Create a free account with them and get your API key here:
 * https://home.openweathermap.org/api_keys
 */
#define OPENWEATHERMAP_API_KEY "your api key here"

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
#define WEATHER_LOCATION "Oklahoma City, Oklahoma, US"

/**
 * The latitude and longitude for weather information. Optional. Overrides WEATHER_LOCATION if set.
 */
// #define WEATHER_LOCATION_LATITUDE 0.0
// #define WEATHER_LOCATION_LONGITUDE 0.0

/**
 * Show what the day's weather will be for the next 12 hours, starting at this hour (in 24-hour time).
 * 9 = 9am, 12 = 12pm, 15 = 3pm, etc
 * 
 * Since openweathermap gives us the weather in 3-hour intervals, and the hours they have data for are based on UTC,
 * the weather shown may not exactly line up with the hour you enter here. For example, in my timezone (America/Chicago),
 * openweathermap returns the weather for 10AM-10PM, even though I really want the weather for 9AM-9PM. So expect this
 * to be up to an hour and a half off depending on where you live.
 */
#define WEATHER_START_HOUR 9

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
#define RESET_PIN       33
#define DC_PIN          23
#define CS_PIN          15
#define BUSY_PIN        27

/**
 * If debug logs should be printed over serial
 */
// #define DEBUG
