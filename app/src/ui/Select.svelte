<script lang="ts">
    import classNames from "classnames";
    import Label from "./Label.svelte";
    import Helper from "./Helper.svelte";
    import type { SelectOptionType } from "./types";

    export let items: SelectOptionType[] = [];
    export let value: string | number;
    export let label: string = "";
    export let help: string = "";
    export let disabled = false;
    export let placeholder: string | undefined = undefined;
</script>

<Label class="space-y-2" disabled={disabled}>
    {#if label}<span class="mb-2">{label}</span>{/if}
    <select
        {...$$restProps}
        disabled={disabled}
        bind:value
        class={classNames(
            "block w-full text-sm p-2.5 text-gray-900 bg-gray-50 border border-gray-300 rounded-lg focus:ring-gray-500 focus:border-gray-500",
            $$restProps.class
        )}
        on:change
        on:contextmenu
        on:input
    >
        {#if placeholder}
            <option disabled selected value="">{placeholder}</option>
        {/if}

        {#each items as { value, name } (value)}
            <option {value}>{name}</option>
        {:else}
            <slot />
        {/each}
    </select>
    {#if help}<Helper>{help}</Helper>{/if}
</Label>
