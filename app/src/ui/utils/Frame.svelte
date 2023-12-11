<script lang="ts">
    import classNames from "classnames";

    export let tag: string = "div";
    export let rounded: boolean = false;
    export let border: boolean = false;
    export let shadow: boolean = false;

    let divClass: string;

    $: divClass = classNames(
        "bg-white text-gray-500",
        rounded && "rounded-lg",
        border && "border border-gray-200",
        shadow && "shadow-md",
        $$props.class
    );
</script>

<svelte:element
    this={tag}
    {...$$restProps}
    class={divClass}
>
    <slot />
</svelte:element>

