<script lang="ts">
    import classNames from "classnames";
    import type { UiColor } from "./types";

    export let helperClass: string = "text-xs font-normal";
    export let color: UiColor = "base";

    const colorClasses: Record<UiColor, string> = {
        base: "text-gray-400",
        green: "text-green-700",
        red: "text-red-700",
        yellow: "text-yellow-700",
    };
</script>

<p
    {...$$restProps}
    class={classNames(helperClass, colorClasses[color], $$props.class)}
>
    <slot />
</p>
