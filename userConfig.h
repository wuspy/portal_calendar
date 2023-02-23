#ifndef PORTALCALENDAR_USERCONFIG_H
#define PORTALCALENDAR_USERCONFIG_H

#include "Arduino.h"
#include "global.h"
#include "WiFiManager.h"

// Forward Declare
class WiFiManager;

enum EWeatherDisplayType
{
	FiveDayForecast = 1,
	TodaysForecast = 2
};

enum EWeatherSecondaryDisplayType
{
	ChanceOfPrecipitation = 1,
	Humidity = 2
};

enum EWeatherUnits
{
	Metric = 1,
	Imperial = 2
};

class UserConfig
{
public:
	UserConfig();
	virtual ~UserConfig();
	
	bool loadFromFilesystem();
	bool saveToFilesystem();
	void addParamsToWiFiManager(WiFiManager& wifiManager);
	void saveParamsFromWiFiManager();
	void createWiFiParams();
	
public:
	// Should we show the day name on the right side (next to the XX/XX day)
	bool bShowDay = true;
	// Should we show the month name on the top
	bool bShowMonth = true;
	// Should we show the year on the top
	bool bShowYear = false;
	// Should we show the weather in place of Chamber Icons
	bool bShowWeather = false;
	// 
	
	/**
	 * The name of your timezone. Requires connection with a 3rd-party service to get all the DST & offset information.
	 * 
	 * You can find a list of timezone names here:
	 * https://en.wikipedia.org/wiki/List_of_tz_database_time_zones
	 */
	String timeZone = "America/Chicago";

	/**
	 * The full POSIX specification for your timezone. If TIME_ZONE is set, this will only serve as a backup to that.
	 * If not, then no 3rd-party timezone services will be used, however this will need to be changed if your timezone
	 * or DST laws change in the future.
	 */
	String posixTimeZone = "CST6CDT,M3.2.0,M11.1.0";

	/**
	 * Your API key for openweathermap.org, which is the service used to get the weather for your location.
	 * This is REQUIRED to use the weather feature. Create a free account with them and get your API key here:
	 * https://home.openweathermap.org/api_keys
	 */
	String openWeatherMapAPIKey = "";

	/**
	 * 1: Show a 5-day weather forecast
	 * 2: Show today's weather forecast in 3-hour intervals
	 */
	EWeatherDisplayType weatherDisplayType = EWeatherDisplayType::FiveDayForecast;
	
	/**
	 * If you choose to show today's weather in 3-hour intervals, this selects what other information is shown under
	 * the temperature (in 5-day forecast mode, the high and low temperature are shown so this isn't an option).
	 * 
	 * 1: Chance of precipitation
	 * 2: Humidity
	 */
	EWeatherSecondaryDisplayType weatherSecondaryDisplayType = EWeatherSecondaryDisplayType::ChanceOfPrecipitation;

	/**
	* If you choose to show today's weather in 3-hour intervals, this will display times in 24-hour format instead of 12-hour.
	*/
	bool bUse24HourTime = false;
	
	/**
	* Metric or Imperial
	*/
	EWeatherUnits weatherUnitDisplay = EWeatherUnits::Imperial;

	/**
	* The location to get weather information for. Can also be a zip code for US locations.
	*/
	String weatherLocation = "Oklahoma City, Oklahoma, US";
	
	/**
	 * The latitude and longitude for weather information. Optional. Overrides WEATHER_LOCATION if set.
	 */
	float weatherLocationOverrideLatitude = 0.0f;
	float weatherLocationOverrideLongitude = 0.0f;
	
	/**
	 * Show what the day's weather will be for the next 12 hours, starting at this hour (in 24-hour time).
	 * 9 = 9am, 12 = 12pm, 15 = 3pm, etc
	 * 
	 * Since openweathermap gives us the weather in 3-hour intervals, and the hours they have data for are based on UTC,
	 * the weather shown may not exactly line up with the hour you enter here. For example, in my timezone (America/Chicago),
	 * openweathermap returns the weather for 10AM-10PM, even though I really want the weather for 9AM-9PM. So expect this
	 * to be up to an hour and a half off depending on where you live.
	 */
	uint8_t weatherStartHour = 9;
	
private:
	// We have to store these parameter classes so that we can fetch them later,
	// after user configuration is complete through the AP.
	WiFiManagerParameter* bShowDayParam;
	WiFiManagerParameter* bShowMonthParam;
	WiFiManagerParameter* bShowYearParam;
	WiFiManagerParameter* bShowWeatherParam;
	WiFiManagerParameter* timeZoneParam;
	// WiFiManagerParameter posixTimeZoneParam;
	WiFiManagerParameter* openWeatherAPIParam;
	WiFiManagerParameter* weatherDisplayTypeParam;
	WiFiManagerParameter* secondaryWeatherInfoParam;
	WiFiManagerParameter* bUse24HourTimeParam;
	WiFiManagerParameter* weatherUnitsParam;
	WiFiManagerParameter* weatherLocationParam;
	WiFiManagerParameter* weatherLocLatParam;
	WiFiManagerParameter* weatherLocLongParam;
	WiFiManagerParameter* weatherStartHourParam;
};

#endif // PORTALCALENDAR_USERCONFIG_H