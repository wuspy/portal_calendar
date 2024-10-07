import type { Preferences, WifiScanResponse, LocationQueryResponse, WifiStatus, WifiInterfaceProperties } from "../model";

// Running in dev mode assumes the calendar is running with DEB_WEBSERVER and is therefore available
// on a local wifi network rather than its own AP. This address can be replaced with the current address
// of the calendar on the local network.
const ROOT_URL = process.env.NODE_ENV === "development" ? "http://192.168.1.14" : "";

namespace Api {
    export type Result<T> = Omit<Response, "json"> & { json(): Promise<T> }

    export const scanWifi = () => fetch<WifiScanResponse[]>({
        url: `${ROOT_URL}/wifi/scan`,
        method: "GET",
    });

    export const connectToWifi = (ssid: string, password?: string) => fetch<WifiStatus>({
        url: `${ROOT_URL}/wifi`,
        method: "POST",
        data: { ssid, password },
        dataEncoding: "form",
    });

    export const fetchWifiStatus = () => fetch<WifiStatus>({
        url: `${ROOT_URL}/wifi`,
        method: "GET",
        timeout: 5000,
    });

    export const forgetWifi = () => fetch<void>({
        url: `${ROOT_URL}/wifi`,
        method: "DELETE",
        timeout: 5000,
    });

    export const fetchWifiInterfaceProperties = () => fetch<WifiInterfaceProperties>({
        url: `${ROOT_URL}/wifi/interface`,
        method: "GET",
        timeout: 5000,
    });

    export const saveWifiInterfaceProperties = (hostname: string) => fetch<void>({
        url: `${ROOT_URL}/wifi/interface`,
        method: "PATCH",
        data: { hostname },
        dataEncoding: "form",
    });

    export const fetchLocales = () => fetch<Record<string, string>>({
        url: `${ROOT_URL}/locales`,
        method: "GET",
        timeout: 5000,
    });

    export const fetchPreferences = () => fetch<Preferences>({
        url: `${ROOT_URL}/preferences`,
        method: "GET",
        timeout: 5000,
    });

    export const savePreferences = (preferences: Partial<Preferences>) => fetch<void>({
        url: `${ROOT_URL}/preferences`,
        method: "PATCH",
        data: preferences,
    });

    export const testWeatherApiKey = (apiKey: string) => fetch<void>({
        url: `${ROOT_URL}/weather/test?appid=${apiKey}`,
        method: "GET",
    });

    export const queryWeatherLocation = (location: string, apiKey: string) => fetch<LocationQueryResponse>({
        url: `${ROOT_URL}/weather/location?q=${location}&appid=${apiKey}`,
        method: "GET",
    });

    export const testNtpServer = (server: string) => fetch<void>({
        url: `${ROOT_URL}/ntp/test?server=${server}`,
        method: "GET",
    });

    export const lookupTimezone = (server: string, timezone: string) => fetch<never>({
        url: `${ROOT_URL}/timezoned/lookup?server=${server}&timezone=${timezone}`,
        method: "GET",
    });

    interface RequestProps {
        url: string;
        method: "GET" | "PUT" | "POST" | "PATCH" | "DELETE";
        timeout?: number;
        data?: object,
        // The only reason this option exists is because I've had memory management problems decoding 
        // JSON bodies in deferred requests that I still haven't figured out how to solve.
        dataEncoding?: "json" | "form",
    }

    async function fetch<T>({ url, method, timeout, data, dataEncoding = "json" }: RequestProps): Promise<Result<T>> {
        const controller = new AbortController();
        const id = setTimeout(() => controller.abort(), timeout || 30000);
        let body: BodyInit | undefined;
        const headers: HeadersInit = {
            "Accept": "application/json,text/plain",
        };
        if (data) {
            if (dataEncoding === "form") {
                const formData = new FormData();
                for (const [key, value] of Object.entries(data)) {
                    formData.append(key, value);
                }
                body = formData;
            } else {
                body = JSON.stringify(data);
                headers["Content-type"] = "application/json";
            }
        }
        const result = await window.fetch(url, { method, body, headers, signal: controller.signal });
        clearTimeout(id);
        return result;
    };
}

export default Api;
