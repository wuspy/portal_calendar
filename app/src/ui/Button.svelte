<script lang="ts">
    import classNames from "classnames";
    import type { ButtonType, ButtonVariant } from "./types";
    import type { HTMLAttributes } from "svelte/elements";
    import type { Snippet } from "svelte";
    import Spinner from "./Spinner.svelte";

    interface Props extends HTMLAttributes<HTMLElement> {
        pill?: boolean;
        variant?: ButtonVariant;
        href?: string;
        type?: ButtonType;
        disabled?: boolean | null;
        loading?: boolean | null;
        children: Snippet;
    }

    let {
        pill = false,
        variant = "contained",
        href = undefined,
        type = "button",
        loading,
        children,
        ...props
    }: Props = $props();

    const backgroundClasses: Record<ButtonVariant, string> = {
        contained: "bg-gray-800 hover:bg-gray-900 focus:ring-gray-300",
        outlined: "border border-gray-800 hover:bg-gray-100 focus:bg-gray-300 focus:ring-gray-300",
        text: "hover:bg-gray-100 focus:bg-gray-300",
    };

    let textClass = $derived(variant === "contained" ? "text-white" : "text-gray-900");
    let buttonClass = $derived(classNames(
        "relative text-center font-medium inline-flex items-center justify-center focus:outline-none px-5 py-2.5 text-sm",
        loading && "button-loading",
        variant !== "text" && "focus:ring-4",
        variant === "contained" ? "text-white" : "text-gray-900",
        textClass,
        backgroundClasses[variant],
        pill ? "rounded-full" : "rounded-lg",
        props.disabled && "cursor-not-allowed opacity-50",
        props.class
    ));
</script>

<svelte:element
    {...props}
    {href}
    this={href ? "a" : "button"}
    type={href ? undefined : type}
    class={buttonClass}
    disabled={loading || props.disabled}
>
    {@render children()}
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
