<script lang="ts">
    import classNames from "classnames";
    import Label from "./Label.svelte";
    import Wrapper from "./utils/Wrapper.svelte";
    import Helper from "./Helper.svelte";
    import type { HTMLInputAttributes } from "svelte/elements";

    interface Props extends Omit<HTMLInputAttributes, "type"> {
        label?: string;
        help?: string;
    }

    let {
        checked = $bindable(),
        disabled,
        label = "",
        help = "",
        ...props
    }: Props = $props();
</script>

<Wrapper class={classNames("space-y-2", props.class)} show={!!help}>
    <Label {disabled} class={classNames("flex items-center", !help && props.class)} show={!!label}>
        <input
            {...props}
            type="checkbox"
            {disabled}
            bind:checked
            class={classNames(
                "w-5 h-5 sm:w-4 sm:h-4 bg-gray-100 border-gray-300 text-gray-900 rounded focus:ring-gray-500 focus:ring-2",
                !!label && "mr-2"
            )}
        />
        {label}
    </Label>
    {#if help}<Helper>{help}</Helper>{/if}
</Wrapper>
