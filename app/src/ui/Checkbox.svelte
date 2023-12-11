<script lang="ts">
    import classNames from "classnames";
    import Label from "./Label.svelte";
    import Wrapper from "./utils/Wrapper.svelte";
    import Helper from "./Helper.svelte";
    // properties forwarding
    export let group: (string | number)[] = [];
    export let value: string | number = "";
    export let checked: boolean | undefined = undefined;
    export let disabled: boolean = false;
    export let label: string = "";
    export let help: string = "";
    // react on external group changes
    function init(_: HTMLElement, _group: (string | number)[]) {
        if (checked === undefined) checked = _group.includes(value);
        onChange();
        return {
            update(_group: (string | number)[]) {
                checked = _group.includes(value);
            },
        };
    }
    function onChange() {
        // There's a bug in Svelte and bind:group is not working with wrapped checkbox
        // This workaround is taken from:
        // https://svelte.dev/repl/de117399559f4e7e9e14e2fc9ab243cc?version=3.12.1
        const index = group.indexOf(value);
        if (checked === undefined) checked = index >= 0;
        if (checked) {
            if (index < 0) {
                group.push(value);
                group = group;
            }
        } else {
            if (index >= 0) {
                group.splice(index, 1);
                group = group;
            }
        }
    }
</script>

<Wrapper class={classNames("space-y-2", $$props.class)} show={!!help}>
    <Label disabled={disabled} class={classNames("flex items-center", !help && $$props.class)} show={!!label}>
        <input
            use:init={group}
            type="checkbox"
            disabled={disabled}
            bind:checked
            on:keyup
            on:keydown
            on:keypress
            on:focus
            on:blur
            on:click
            on:mouseover
            on:mouseenter
            on:mouseleave
            on:paste
            on:change={onChange}
            on:change
            {value}
            {...$$restProps}
            class={classNames(
                "w-5 h-5 sm:w-4 sm:h-4 bg-gray-100 border-gray-300 text-gray-900 rounded focus:ring-gray-500 focus:ring-2",
                !!label && "mr-2"
            )}
        />
        {label}
    </Label>
    {#if help}<Helper>{help}</Helper>{/if}
</Wrapper>
