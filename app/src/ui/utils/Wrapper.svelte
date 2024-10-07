<script lang="ts">
    import type { HTMLAttributes } from "svelte/elements";
    import type { Snippet } from "svelte";

    interface Props extends HTMLAttributes<HTMLDivElement> {
        tag?: string;
        show: boolean;
        children: Snippet;
    }

    let {
        tag = "div",
        show,
        children,
        ...props
    }: Props = $props();

</script>

{#if tag}
    <svelte:element {...props} this={tag}>{@render children()}</svelte:element>
{:else}
    {@render children()}
{/if}
