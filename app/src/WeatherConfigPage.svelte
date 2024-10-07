<script context="module" lang="ts">
    // Caches known locations to prevent unnecessary requests to owm
    const validApiKeys: string[] = [];
    const knownLocations: Record<string, LocationQueryResponse> = {};
</script>

<script lang="ts">
    import { fly } from "svelte/transition";
    import Api from "./api";
    import { WeatherDisplayType, WeatherSecondaryInfo, WeatherUnits, type LocationQueryResponse} from "./model";
    import { openSaveFailedModal } from "./SaveFailedModal.svelte";
    import { preferences, savePreferences } from "./store";
    import { Checkbox, Input, PasswordInput, Select, type SelectOptionType, type InputValidator, WizardPageLayout } from "./ui";

    let weatherEnabled = $state($preferences.weatherEnabled);
    let weatherLocName = $state($preferences.weatherLocName);
    let owmApiKey = $state($preferences.owmApiKey);
    let weatherLat = $state($preferences.weatherLat);
    let weatherLon = $state($preferences.weatherLon);
    let weatherDisplay = $state($preferences.weatherDisplay);
    let weatherUnits = $state($preferences.weatherUnits);
    let weatherInfo = $state($preferences.weatherInfo);
    let weatherStartHr = $state($preferences.weatherStartHr);
    let show24Hr = $state($preferences.show24Hr);

    interface Props {
        onBack: () => void;
        onNext: () => void;
    }

    let { onBack, onNext }: Props = $props();

    let saving = $state(false);

    const validateLocation: InputValidator = async (location: string) => {
        location = location.trim();
        if (knownLocations[location]) {
            const { lat, lon, name } = knownLocations[location];
            weatherLat = lat;
            weatherLon = lon;
            return [true, name];
        }
        weatherLat = weatherLon = 0;
        try {
            const response = await Api.queryWeatherLocation(location, owmApiKey);
            switch (response.status) {
                case 200:
                    knownLocations[location] = await response.json();
                    return await validateLocation(location);
                case 400:
                case 401:
                    return [false, "Enter a valid API key first"];
                case 404:
                    return [false, "Location not found"];
                default:
                    return [false, "Error communicating with OpenWeatherMap, try again later"];
            }
        } catch (e) {
            return [false, "Error communicating with calendar"];
        }
    };

    const validateApiKey: InputValidator = async (owmApiKey: string) => {
        owmApiKey = owmApiKey.trim();
        if (validApiKeys.includes(owmApiKey)) {
            return [true, "Verified"];
        }
        try {
            const response = await Api.testWeatherApiKey(owmApiKey);
            switch (response.status) {
                case 200:
                    validApiKeys.push(owmApiKey);
                    return await validateApiKey(owmApiKey);
                case 400:
                case 401:
                    return [false, "Invalid API key"];
                default:
                    return [false, "Error communicating with OpenWeatherMap, try again later"];
            }
        } catch (e) {
            return [false, "Error communicating with calendar"];
        }
    };

    const displayOptions: SelectOptionType[] = [
        {value: WeatherDisplayType.Forecast5Day, name: "5-day forecast"},
        {value: WeatherDisplayType.Forecast12Hour, name: "Today's forecast"},
    ];

    const secondaryInformationOptions: SelectOptionType[] = [
        {value: WeatherSecondaryInfo.PoP, name: "Chance of precipitation"},
        {value: WeatherSecondaryInfo.Humidity, name: "Humidity"},
    ];

    const unitsOptions: SelectOptionType[] = [
        {value: WeatherUnits.Imperial, name: "Imperial"},
        {value: WeatherUnits.Metric, name: "Metric"},
    ];

    const format24HourTime = (hour: number) => `${hour.toString().padStart(2, "0")}:00`;
    const format12HourTime = (hour: number) => {
        const amPm = hour > 11 ? "PM" : "AM";
        let hour12 = hour % 12;
        if (hour12 === 0) {
            hour12 = 12;
        }
        return `${hour12}:00 ${amPm}`;
    };

    let startingHourOptions: SelectOptionType[] = $derived([...Array(24).keys()].map(start => {
        const end = (start + 12) % 24;
        return {
            value: start,
            name: show24Hr
                ? `${format24HourTime(start)} - ${format24HourTime(end)}`
                : `${format12HourTime(start)} - ${format12HourTime(end)}`
        };
    }));

    async function saveAndContinue() {
        saving = true;
        if (await savePreferences({
            weatherEnabled,
            weatherLocName,
            owmApiKey,
            weatherLat,
            weatherLon,
            weatherDisplay,
            weatherUnits,
            weatherInfo,
            weatherStartHr,
            show24Hr,
        })) {
            onNext();
        } else {
            saving = false;
            openSaveFailedModal();
        }
    }
</script>

<WizardPageLayout
    title="Weather"
    {onBack}
    onNext={saveAndContinue}
    {saving}
    nextDisabled={weatherEnabled && (!owmApiKey || (!weatherLat && !weatherLon))}
>
    <Checkbox
        label="Enable weather integration"
        bind:checked={weatherEnabled}
        help="Show a weather forecast in place of the chamber icons. You can switch between the two at any time using the MODE button."
    />
    {#if weatherEnabled}
        <span class="border-t" ></span>
        <div class="space-y-6" in:fly="{{y: -20, duration: 200}}">
            <PasswordInput
                label="OpenWeatherMap API Key"
                placeholder="enter your API key"
                bind:value={owmApiKey}
                validator={validateApiKey}
                validateOnMount={!!owmApiKey}
                required
                help={"Your API key for openweathermap.org, which is the service used to get the weather for your location."
                    + " This is required to use the weather feature. Create a free account with them and get your API key at"
                    + " https://home.openweathermap.org/api_keys."}
            />
            <Input
                label="Weather Location"
                placeholder="enter the location you want the weather for"
                bind:value={weatherLocName}
                validator={validateLocation}
                validateOnMount={!!weatherLocName}
                required
            />
            <Select
                label="Weather Display"
                bind:value={weatherDisplay}
                items={displayOptions}
            />
            <Select
                label="Secondary Information"
                disabled={weatherDisplay === WeatherDisplayType.Forecast5Day}
                bind:value={weatherInfo}
                items={secondaryInformationOptions}
                help="Only available when viewing today's forecast. In the 5-day forecast both the high and low daily temperatures are shown."
            />
            <Select
                label="Weather Units" 
                bind:value={weatherUnits}
                items={unitsOptions}
            />
            <Select
                label="Time"
                bind:value={weatherStartHr}
                items={startingHourOptions}
                help={"Show what the day's weather will be during this time. Since OpenWeatherMap only has data in 3-hour intervals,"
                    + " and their hours are based on UTC, the time selected here may not exactly line up with what is shown depending"
                    + " on your location"}
            />
            <Checkbox
                label="Use 24-hour time"
                bind:checked={show24Hr}
            />
        </div>
    {/if}
</WizardPageLayout>
