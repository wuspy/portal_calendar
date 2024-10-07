<script lang="ts">
    import classNames from "classnames";
    import Frame from "./utils/Frame.svelte";
    import type { ComponentProps, Snippet } from "svelte";

    interface Props extends Omit<ComponentProps<Frame>, "rounded" | "border" | "shadow"> {
        horizontal?: boolean;
        reverse?: boolean;
        children: Snippet;
    }

    let {
        horizontal = false,
        reverse = false,
        children,
        ...props
    }: Props = $props();
    
    let cardClass = $derived(classNames(
        "flex",
        reverse ? "flex-col-reverse" : "flex-col",
        horizontal &&
            (reverse
                ? "md:flex-row-reverse md:max-w-xl"
                : "md:flex-row md:max-w-xl"),
        props.class
    ));
</script>

<Frame
    {...props}
    rounded
    border
    shadow
    class={cardClass}
>
    {@render children()}
</Frame>
