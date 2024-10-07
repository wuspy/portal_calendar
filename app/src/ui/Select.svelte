<script lang="ts">
    import classNames from "classnames";
    import Label from "./Label.svelte";
    import Helper from "./Helper.svelte";
    import type { SelectOptionType } from "./types";
    import type { HTMLSelectAttributes } from "svelte/elements";

    interface Props extends HTMLSelectAttributes {
        items?: SelectOptionType[];
        value: string | number;
        label?: string;
        help?: string;
    }

    let {
        items = [],
        value = $bindable(),
        label = "",
        help = "",
        disabled,
        children,
        ...props
    }: Props = $props();

    const colorClass = "bg-gray-50 border-gray-300 focus:border-gray-500 focus:ring-gray-500";
    const textColorClass = "text-gray-900 placeholder-gray-400";
    const disabledClass = "disabled:cursor-not-allowed disabled:opacity-50";
</script>

<Label class="space-y-2" {disabled} show={!!label || !!help}>
    {#if label}<span class="mb-2">{label}</span>{/if}
    <select
        {...props}
        {disabled}
        bind:value
        class={classNames(
            "block w-full text-sm p-2.5 border rounded-lg",
            colorClass,
            textColorClass,
            disabledClass,
            props.class
        )}
    >
        {#each items as { value, name } (value)}
            <option {value}>{name}</option>
        {:else}
            {#if children}{@render children()}{/if}
        {/each}
    </select>
    {#if help}<Helper>{help}</Helper>{/if}
</Label>
