<script lang="ts">
    import type { SizeType, UiColor } from "./types";
    import classNames from "classnames";
    import type { IconType } from "./SvgIcon.svelte";
    import SvgIcon from "./SvgIcon.svelte";

    export let color: UiColor = "base";
    export let name: string | undefined = undefined;
    export let size: SizeType = "md";
    export let icon: IconType;

    const colors: Record<UiColor, string> = {
        base: "focus:ring-gray-400 hover:bg-gray-100",
        red: "text-red-500 focus:ring-red-400 hover:bg-red-200",
        green: "text-green-500 focus:ring-green-400 hover:bg-green-200",
        yellow: "text-yellow-500 focus:ring-yellow-400 hover:bg-yellow-200",
    };
    const sizing: Record<SizeType, string> = {
        xs: "m-0.5 rounded focus:ring-1 p-0.5",
        sm: "m-0.5 rounded focus:ring-1 p-1",
        md: "rounded-lg focus:ring-2 p-1.5",
        lg: "rounded-lg focus:ring-2 p-2",
        xl: "rounded-lg focus:ring-2 p-3",
    };
    let buttonClass: string;
    $: buttonClass = classNames(
        "focus:outline-none whitespace-normal",
        sizing[size],
        colors[color],
        $$props.class
    );
</script>

<button
    on:click
    type="button"
    {...$$restProps}
    class={buttonClass}
    aria-label={name}
>
    <SvgIcon {icon} {size} title={name} />
</button>
