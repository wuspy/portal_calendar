<script lang="ts">
    import { getContext, type Snippet } from "svelte";
    import classNames from "classnames";
    import type { HTMLAttributes } from "svelte/elements";

    interface Props extends HTMLAttributes<HTMLElement> {
        active?: boolean;
        current?: boolean;
        disabled?: boolean;
        children: Snippet;
    }

    let {
        active = getContext("active"),
        current = false,
        disabled = false,
        children,
        ...props
    }: Props = $props();

    const states = {
        current: "text-white bg-gray-700",
        normal: "text-gray-800",
        disabled: "text-gray-900 bg-gray-100",
    };
    let focusClass = "focus:z-40 focus:outline-none focus:ring-2 focus:ring-gray-700 focus:text-gray-700";
    let hoverClass = "hover:bg-gray-100 hover:text-gray-700";
    let state: "disabled" | "current" | "normal" = $derived(disabled ? "disabled" : current ? "current" : "normal");
    let itemClass = $derived(classNames(
        "p-4 w-full text-sm font-medium",
        "first:rounded-t-lg last:rounded-b-lg",
        states[state],
        active && state === "disabled" && "cursor-not-allowed",
        active && state === "normal" && hoverClass,
        active && state === "normal" && focusClass,
        props.class
    ));
</script>

{#if !active}
    <li {...props} class={itemClass}>
        {@render children()}
    </li>
{:else}
    <button
        {...props}
        type="button"
        class="inline-flex relative items-center text-left {itemClass}"
        {disabled}
        aria-current={current}
    >
        {@render children()}
    </button>
{/if}
