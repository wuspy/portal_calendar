<script lang="ts">
    import { getContext } from "svelte";
    import classNames from "classnames";
    export let active: boolean = getContext("active");
    export let current: boolean = false;
    export let disabled: boolean = false;
    const states = {
        current: "text-white bg-gray-700",
        normal: "text-gray-800",
        disabled: "text-gray-900 bg-gray-100",
    };
    let focusClass = "focus:z-40 focus:outline-none focus:ring-2 focus:ring-gray-700 focus:text-gray-700";
    let hoverClass = "hover:bg-gray-100 hover:text-gray-700";
    let state: "disabled" | "current" | "normal";
    $: state = disabled ? "disabled" : current ? "current" : "normal";
    let itemClass: string;
    $: itemClass = classNames(
        "p-4 w-full text-sm font-medium",
        "first:rounded-t-lg last:rounded-b-lg",
        states[state],
        active && state === "disabled" && "cursor-not-allowed",
        active && state === "normal" && hoverClass,
        active && state === "normal" && focusClass,
        $$props.class
    );
</script>

{#if !active}
    <li class={itemClass}>
        <slot item={$$props} />
    </li>
{:else}
    <button
        type="button"
        class="inline-flex relative items-center text-left {itemClass}"
        {disabled}
        on:blur
        on:change
        on:click
        on:focus
        on:keydown
        on:keypress
        on:keyup
        on:mouseenter
        on:mouseleave
        on:mouseover
        aria-current={current}
    >
        <slot item={$$props} />
    </button>
{/if}
