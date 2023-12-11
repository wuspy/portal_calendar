import { readonly, writable } from "svelte/store";
import Api from "../api";

const _locales = writable<Record<string, string>>();

export async function loadLocales() {
    const result = await Api.fetchLocales();
    if (result.status === 200) {
        _locales.set(await result.json());
    } else {
        throw result.status;
    }
}

export const locales = readonly(_locales);
