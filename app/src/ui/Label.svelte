<script lang="ts">
    import classNames from "classnames";
    export let disabled = false;
    export let show: boolean = true;
</script>

{#if show}
    <!-- svelte-ignore a11y-label-has-associated-control -->
    <label
        {...$$restProps}
        class={classNames(
            "text-sm font-medium block",
            disabled ? "text-gray-400" : "text-gray-900",
            $$restProps.class
        )}
    >
        <slot />
    </label>
{:else}
    <slot />
{/if}
