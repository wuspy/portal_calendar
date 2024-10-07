<script lang="ts">
    import classNames from "classnames";
    import type { Snippet } from "svelte";
    import type { HTMLAttributes } from "svelte/elements";

    interface Props extends HTMLAttributes<HTMLDivElement> {
        tag?: string;
        rounded?: boolean;
        border?: boolean;
        shadow?: boolean;
        children: Snippet;
    }

    let {
        tag = "div",
        rounded = false,
        border = false,
        shadow = false,
        children,
        ...props
    }: Props = $props();

    let divClass = $derived(classNames(
        "bg-white text-gray-500",
        rounded && "rounded-lg",
        border && "border border-gray-200",
        shadow && "shadow-md",
        props.class
    ));
</script>

<svelte:element
    {...props}
    this={tag}
    class={divClass}
>
    {@render children()}
</svelte:element>
