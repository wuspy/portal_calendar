<script lang="ts">
    import type { HTMLLabelAttributes } from "svelte/elements";
    import classNames from "classnames";
    import type { Snippet } from "svelte";

    interface Props extends HTMLLabelAttributes {
        show?: boolean;
        disabled?: boolean | null;
        children: Snippet;
    }

    let { show = true, disabled = false, children, ...props }: Props = $props();
</script>

{#if show}
    <!-- svelte-ignore a11y_label_has_associated_control -->
    <label
        {...props}
        class={classNames(
            "text-sm font-medium block",
            disabled ? "text-gray-400" : "text-gray-900",
            props.class
        )}
    >
        {@render children()}
    </label>
{:else}
    {@render children()}
{/if}
