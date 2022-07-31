#define WIFI_NAME "name"
#define WIFI_PASS "password"

#define SHOW_DAY
#define SHOW_MONTH
// #define SHOW_YEAR

/**
 * If timezone should be looked up based on IP address using an 3rd-party service.
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

#define NTP_SERVERS "pool.ntp.org", "time.google.com"

#define SPI_BUS         HSPI
#define RESET_PIN       2
#define DC_PIN          4
#define CS_PIN          15
#define BUSY_PIN        5
