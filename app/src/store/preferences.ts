import { readonly, writable } from "svelte/store";
import Api from "../api";
import type { Preferences } from "../model";

const _preferences = writable<Preferences>();

export async function loadPreferences() {
    const result = await Api.fetchPreferences();
    if (result.status === 200) {
        _preferences.set(await result.json());
    } else {
        throw result.status;
    }
}

export async function savePreferences(preferences: Partial<Preferences>) {
    try {
        const result = await Api.savePreferences(preferences);
        if (result.status === 200) {
            _preferences.update(x => ({...x, ...preferences}));
            return true;
        }
    } catch { }
    return false;
}

export const preferences = readonly(_preferences);
