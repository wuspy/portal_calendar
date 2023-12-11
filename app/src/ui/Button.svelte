<script lang="ts">
    import classNames from "classnames";
    import Spinner from "./Spinner.svelte";
    import type { ButtonType, ButtonVariant } from "./types";

    export let pill: boolean = false;
    export let variant: ButtonVariant = "contained";
    export let href: string | undefined = undefined;
    export let type: ButtonType = "button";
    export let loading = false;

    const backgroundClasses: Record<ButtonVariant, string> = {
        contained: "bg-gray-800 hover:bg-gray-900 focus:ring-gray-300",
        outlined: "border border-gray-800 hover:bg-gray-100 focus:bg-gray-300 focus:ring-gray-300",
        text: "hover:bg-gray-100 focus:bg-gray-300",
    };

    let textClass: string;
    $: textClass = variant === "contained" ? "text-white" : "text-gray-900";

    let buttonClass: string;
    $: buttonClass = classNames(
        "relative text-center font-medium inline-flex items-center justify-center focus:outline-none px-5 py-2.5 text-sm",
        loading && "button-loading",
        variant !== "text" && "focus:ring-4",
        variant === "contained" ? "text-white" : "text-gray-900",
        textClass,
        backgroundClasses[variant],
        pill ? "rounded-full" : "rounded-lg",
        $$props.disabled && "cursor-not-allowed opacity-50",
        $$props.class
    );
</script>

<svelte:element
    this={href ? "a" : "button"}
    type={href ? undefined : type}
    {href}
    {...$$restProps}
    class={buttonClass}
    disabled={loading || $$props.disabled}
    on:click
    on:change
    on:keydown
    on:keyup
    on:mouseenter
    on:mouseleave
>
    <slot />
    {#if loading}
        <div class="button-spinner absolute {textClass}">
            <Spinner width=24 height=24 />
        </div>
    {/if}
</svelte:element>

<style>
   .button-loading, .button-loading > :not(.button-spinner) {
        color: transparent;
    }
</style>
