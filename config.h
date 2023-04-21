/**
 * Default values for settings
 */
#define DEFAULT_LOCALE "en"

#define DEFAULT_SHOW_DAY true
#define DEFAULT_SHOW_MONTH true
#define DEFAULT_SHOW_YEAR false

#define DEFAULT_HOSTNAME "portalcalendar"

#define DEFAULT_PRIMARY_NTP_SERVER "pool.ntp.org"
#define DEFAULT_SECONDARY_NTP_SERVER "time.google.com"

#define DEFAULT_PRIMARY_TIMEZONED_SERVER "timezoned.rop.nl"
#define DEFAULT_SECONDARY_TIMEZONED_SERVER ""

#define DEFAULT_WEATHER_START_HOUR 9
#define DEFAULT_USE_24H_TIME false

#define DEFAULT_2_NTP_SYNCS_PER_DAY true
#define DEFAULT_MAX_RTC_CORRECTION_FACTOR 0.025

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
 * SSID for the configuration access point
 */
#define AP_SSID "PortalCalendar"

/**
 * Password for the configuration access point. By default, a random 8-digit number is used for the password. Enabling this will make
 * it use the same password every time.
 */
// #define AP_PASS "12345678"

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
 * Enables the internal pullup on GPIO0 (Mode button) for boards that don't have a external pullup resistor on that pin.
 * The EzSBC does have one. If you're using a different board and it bootloops with weather enabled then this option is needed.
 * This may increase power usage in deep sleep.
 */
// #define ENABLE_GPIO0_PULLUP

/**
 * If debug logs should be printed over serial
 */
// #define DEBUG

/**
 * Normally the config server will start if the ESP32 is reset while plugged into USB.
 * This prevents that behavior so that the logs printed by DEBUG can be inspected.
 */
// #define DISABLE_MANUAL_CONFIG_SERVER_ACTIVATION

/**
 * Run the webserver connected to a local WiFi network instead of creating an AP. This allows easily testing
 * the website, but connecting to WiFi won't be available and will simply return mock responses based on the
 * WiFi network it's already connected to.
 */
// #define DEV_WEBSERVER
// #define DEV_WEBSERVER_WIFI_SSID "ssid"
// #define DEV_WEBSERVER_WIFI_PASS "password"
