import { writable, readonly, type Readable } from "svelte/store";
import Api from "../api";
import type { WifiStatus, WifiInterfaceProperties } from "../model";

const _interfaceProperties = writable<WifiInterfaceProperties>();
export const _wifiStatus = writable<WifiStatus>();

export const enum WifiConnectionError {
    AlreadyConnecting,
    NotFound,
    RequestError,
    Other,
}

export async function loadWifiStatus() {
    const result = await Api.fetchWifiStatus();
    if (result.status === 200) {
        _wifiStatus.set(await result.json());
    } else {
        throw result.status;
    }
}

export async function loadWifiInterfaceProperties() {
    const result = await Api.fetchWifiInterfaceProperties();
    if (result.status === 200) {
        _interfaceProperties.set(await result.json());
    } else {
        throw result.status;
    }
}

export async function saveWifiInterfaceProperties(hostname: string) {
    try {
        const result = await Api.saveWifiInterfaceProperties(hostname);
        if (result.status === 200) {
            _interfaceProperties.update(x => ({...x, hostname}));
            return true;
        }
    } catch { }
    return false;
}

export async function connectToWifi(ssid: string, password?: string) {
    let result;
    try {
        result = await Api.connectToWifi(ssid, password);
    } catch (e) {
        throw WifiConnectionError.RequestError;
    }

    switch (result.status) {
        case 200:
            _wifiStatus.set(await result.json());
            break;
        case 409:
            throw WifiConnectionError.AlreadyConnecting;
        case 404:
            throw WifiConnectionError.NotFound;
        default:
            throw WifiConnectionError.Other;
    }
}

export const interfaceProperties: Readable<WifiInterfaceProperties> = readonly(_interfaceProperties);
export const wifiStatus: Readable<WifiStatus> = readonly(_wifiStatus);
