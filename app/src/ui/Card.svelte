<script lang="ts">
    import classNames from "classnames";
    import Frame from "./utils/Frame.svelte";
    export let horizontal: boolean = false;
    export let reverse: boolean = false;
    let cardClass: string;
    $: cardClass = classNames(
        "flex",
        reverse ? "flex-col-reverse" : "flex-col",
        horizontal &&
            (reverse
                ? "md:flex-row-reverse md:max-w-xl"
                : "md:flex-row md:max-w-xl"),
        $$props.class
    );
</script>

<Frame
    rounded
    border
    shadow
    {...$$restProps}
    class={cardClass}
>
    <slot />
</Frame>
