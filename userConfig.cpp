#include "userConfig.h"
#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>  

#define JSON_CONFIG_FILE "/user_config.json"

UserConfig::UserConfig()
{}

UserConfig::~UserConfig()
{
	delete bShowDayParam;
	delete bShowMonthParam;
	delete bShowYearParam;
	delete bShowWeatherParam;
	delete timeZoneParam;
	delete openWeatherAPIParam;
	delete weatherDisplayTypeParam;
	delete secondaryWeatherInfoParam;
	delete bUse24HourTimeParam;
	delete weatherUnitsParam;
	delete weatherLocationParam ;
	delete weatherLocLatParam ;
	delete weatherLocLongParam;
	delete weatherStartHourParam;
}

const char* weatherUnitToString(EWeatherUnits inUnit)
{
	switch(inUnit)
	{
		case EWeatherUnits::Metric:
		return "Metric";
		case EWeatherUnits::Imperial:
		return "Imperial";
	}
	
	return "";
}

const char* boolToShortString(bool inValue)
{
	return inValue ? "t" : "f";
}

void UserConfig::createWiFiParams()
{
	// ToDo: "Advance" settings like posixTimeZone

	// This should be called after loadFromFilesystem so that the values we put into the UI reflect
	// the user's previous configuration.
	bShowDayParam = new WiFiManagerParameter("bShowDay", "Show Day", boolToShortString(bShowDay), 1);
	bShowMonthParam = new WiFiManagerParameter("bShowMonth", "Show Month", boolToShortString(bShowMonth), 1);
	bShowYearParam = new WiFiManagerParameter("bShowYear", "Show Year", boolToShortString(bShowYear), 1);
	bShowWeatherParam = new WiFiManagerParameter("bShowWeather", "Show Weather (Replaces Chamber Icons)", boolToShortString(bShowWeather), 5);
	timeZoneParam = new WiFiManagerParameter("timeZone", "Timezone (see TZ Database Name: https://en.wikipedia.org/wiki/List_of_tz_database_time_zones)", timeZone.c_str(), 50);
	openWeatherAPIParam = new WiFiManagerParameter("openweatherAPI", "OpenWeather API Key", openWeatherMapAPIKey.c_str(), 50);
	weatherDisplayTypeParam = new WiFiManagerParameter("weatherDisplayType", "Weather Type:\n1: 5-Day Forecast\n2: 3-Hour Intervals", String((int)weatherDisplayType).c_str(), 1);
	secondaryWeatherInfoParam = new WiFiManagerParameter("secondaryWeatherInfo", "Secondary Weather Info (if using 3-Hour Intervals):\n1: Chance of Precipitation\n2: Humidity", String((int)weatherSecondaryDisplayType).c_str(), 1);
	bUse24HourTimeParam = new WiFiManagerParameter("bUse24HourTime", "Use 24Hr Time (if using 3-Hour Intervals)", boolToShortString(bUse24HourTime), 5);
	weatherUnitsParam = new WiFiManagerParameter("weatherUnits", "Metric or Imperial", weatherUnitToString(weatherUnitDisplay), 8);
	weatherLocationParam = new WiFiManagerParameter("weatherLocation", "Location (City, State, Country) or ZipCode (in US)", weatherLocation.c_str(), 50);
	weatherLocLatParam = new WiFiManagerParameter("weatherLocLat", "Location (Latitude) if Location is blank", String(weatherLocationOverrideLatitude).c_str(), 12);
	weatherLocLongParam = new WiFiManagerParameter("weatherLocLong", "Location (Longitude) if Location 	is blank", String(weatherLocationOverrideLongitude).c_str(), 12);
	weatherStartHourParam = new WiFiManagerParameter("weatherStartHour", "When to update Weather Forecast for the Day? [0-24]", String(weatherStartHour).c_str(), 2);
}

bool UserConfig::loadFromFilesystem()
{
  DEBUG_PRINT("Loading Filesystem to load config...");
  
  const bool bFormatIfFailed = true;
  if(SPIFFS.begin(bFormatIfFailed))
  {
    DEBUG_PRINT("SPIFFS Begin Succeeded.");
    if(SPIFFS.exists(JSON_CONFIG_FILE))
    {
      DEBUG_PRINT("Config File found on Filesystem, loading...");
      File configFile = SPIFFS.open(JSON_CONFIG_FILE, "r");
      if(configFile)
      {
        DEBUG_PRINT("Successfully opened Config File");
        StaticJsonDocument<512> json;
        DeserializationError error = deserializeJson(json, configFile);
        if(!error)
        {
          bShowDay = json["bShowDay"].as<bool>();
		  bShowMonth = json["bShowMonth"].as<bool>();
		  bShowYear = json["bShowYear"].as<bool>();
		  bShowWeather = json["bShowWeather"].as<bool>();
		  timeZone = json["timeZone"].as<String>();
		  posixTimeZone = json["posixTimeZone"].as<String>();
		  openWeatherMapAPIKey = json["openweathermapAPI"].as<String>();
		  weatherDisplayType = (EWeatherDisplayType)json["weatherDisplayType"].as<int>();
		  weatherSecondaryDisplayType = (EWeatherSecondaryDisplayType)json["weatherSecondaryDisplayType"].as<int>();
		  bUse24HourTime = json["bUse24HourTime"].as<bool>();
		  weatherUnitDisplay = (EWeatherUnits)json["weatherUnitDisplay"].as<int>();
		  weatherLocation = json["weatherLocation"].as<String>();
		  weatherLocationOverrideLatitude = json["weatherOverrideLat"].as<float>();
		  weatherLocationOverrideLongitude = json["weatherOverrideLong"].as<float>();
		  weatherStartHour = json["weatherStartHour"].as<int>();
          return true;
        }
        else
        {
          DEBUG_PRINT("Failed to deserialized json.");
        }
      }
      else
      {
        DEBUG_PRINT("Failed to open config file");
      }
    }
	else
	{
		DEBUG_PRINT("Config File not created yet, using default values.");
		return false;
	}
  }
  else
  {
    DEBUG_PRINT("Failed to open SPIFFS filesystem at %s", JSON_CONFIG_FILE);
  }

  return false;
}

bool UserConfig::saveToFilesystem()
{
	DEBUG_PRINT("Saving config to Filesystem...");
	StaticJsonDocument<512> json;
	json["bShowDay"] = bShowDay;
	json["bShowMonth"] = bShowMonth;
	json["bShowYear"] = bShowYear;
	json["bShowWeather"] = bShowWeather;
	json["timeZone"] = timeZone;
	json["posixTimeZone"] = posixTimeZone;
	json["openweathermapAPI"] = openWeatherMapAPIKey;
	json["weatherDisplayType"] = (int)weatherDisplayType;
	json["weatherSecondaryDisplayType"] = (int)weatherSecondaryDisplayType;
	json["bUse24HourTime"] = bUse24HourTime;
	json["weatherUnitDisplay"] = (int)weatherUnitDisplay;
	json["weatherLocation"] = weatherLocation;
	json["weatherOverrideLat"] = weatherLocationOverrideLatitude;
	json["weatherOverrideLong"] = weatherLocationOverrideLongitude;
	json["weatherStartHour"] = weatherStartHour;
	
	const bool bFormatIfFailed = true;
	if(SPIFFS.begin(bFormatIfFailed))
	{
		File configFile = SPIFFS.open(JSON_CONFIG_FILE, "w");
		bool bSuccess = false;
		if(configFile)
		{
			if(serializeJson(json, configFile) == 0)
			{
				bSuccess = true;
			}
			else
			{
			  DEBUG_PRINT("Failed to write file.");
			}
		}
		else
		{
			DEBUG_PRINT("Failed to open Config File: %s for writing...", JSON_CONFIG_FILE);
		}

		configFile.close();
		return bSuccess;
	}
	else
	{
		DEBUG_PRINT("Failed to begin SPIFFS Filesystem");
	}
	
	return false;
}

void UserConfig::addParamsToWiFiManager(WiFiManager& wifiManager)
{
	wifiManager.addParameter(bShowDayParam);
	wifiManager.addParameter(bShowMonthParam);
	wifiManager.addParameter(bShowYearParam);
	wifiManager.addParameter(bShowWeatherParam);
	wifiManager.addParameter(timeZoneParam);
	wifiManager.addParameter(openWeatherAPIParam);
	wifiManager.addParameter(weatherDisplayTypeParam);
	wifiManager.addParameter(secondaryWeatherInfoParam);
	wifiManager.addParameter(bUse24HourTimeParam);
	wifiManager.addParameter(weatherUnitsParam);
	wifiManager.addParameter(weatherLocationParam);
	wifiManager.addParameter(weatherLocLatParam);
	wifiManager.addParameter(weatherLocLongParam);
	wifiManager.addParameter(weatherStartHourParam);
}

void UserConfig::saveParamsFromWiFiManager()
{
	bShowDay = bShowDayParam->getValue() == "t";
	bShowMonth = bShowMonthParam->getValue() == "t";
	bShowYear = bShowYearParam->getValue() == "t";
	bShowWeather = bShowWeatherParam->getValue() == "t";
	timeZone = timeZoneParam->getValue();
	openWeatherMapAPIKey = openWeatherAPIParam->getValue();
	
	weatherDisplayType = (EWeatherDisplayType)(atoi(weatherDisplayTypeParam->getValue()));
	weatherSecondaryDisplayType = (EWeatherSecondaryDisplayType)(atoi(secondaryWeatherInfoParam->getValue()));
	bUse24HourTime = bShowDayParam->getValue() == "t";
	weatherUnitDisplay = (EWeatherUnits)(atoi(weatherUnitsParam->getValue()));
	weatherLocation = weatherLocationParam->getValue();
	weatherLocationOverrideLatitude = atof(weatherLocLatParam->getValue());
	weatherLocationOverrideLongitude = atof(weatherLocLongParam->getValue());
	weatherStartHour = atoi(weatherStartHourParam->getValue());
	
	saveToFilesystem();
}