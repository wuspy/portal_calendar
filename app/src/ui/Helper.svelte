<script lang="ts">
    import classNames from "classnames";
    import type { UiColor } from "./types";
    import type { HTMLAttributes } from "svelte/elements";
    import type { Snippet } from "svelte";

    interface Props extends HTMLAttributes<HTMLParagraphElement> {
        color?: UiColor;
        children: Snippet;
    }

    let { color = "base", children, ...props }: Props = $props();

    const colorClasses: Record<UiColor, string> = {
        base: "text-gray-400",
        green: "text-green-700",
        red: "text-red-700",
        yellow: "text-yellow-700",
    };
</script>

<p
    {...props}
    class={classNames("text-xs font-normal", colorClasses[color], props.class)}
>
    {@render children()}
</p>
