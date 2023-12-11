export const enum WeatherDisplayType {
    Forecast5Day = 0,
    Forecast12Hour = 1,
}

export const enum WeatherSecondaryInfo {
    PoP = 0,
    Humidity = 1,
}

export const enum WeatherUnits {
    Imperial = 0,
    Metric = 1,
}

export interface Preferences {
    showDay: boolean;
    showMonth: boolean;
    showYear: boolean;
    timezone: string;
    locale: string;
    twoNtpSyncs: boolean;
    rtcCorrection: number;
    ntpServer1: string;
    ntpServer2: string;
    tzdServer1: string;
    tzdServer2: string;
    weatherEnabled: boolean;
    owmApiKey: string;
    weatherLocName: string;
    weatherLat: number;
    weatherLon: number;
    weatherDisplay: WeatherDisplayType;
    weatherInfo: WeatherSecondaryInfo;
    weatherUnits: WeatherUnits;
    weatherStartHr: number;
    show24Hr: boolean;
}

export interface WifiScanResponse {
    ssid: string;
    rssi: number;
    open: boolean;
}

export interface LocationQueryResponse {
    lat: number;
    lon: number;
    name: string;
}

export type WifiStatus = {
    connected: false;
    ssid: string;
} | {
    connected: true;
    ssid: string;
    ip: string;
    rssi: number;
    gateway: string;
    mask: string;
    dns0: string;
    dns1: string;
    inUse: boolean;
};

export interface WifiInterfaceProperties {
    hostname: string;
    mac: string;
}
