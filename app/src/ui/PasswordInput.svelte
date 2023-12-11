<script lang="ts">
    import Input from "./Input.svelte";
    import SvgIcon from "./SvgIcon.svelte";

    export let valueShown = false;
    export let value: string | number = "";
    export let disabled: boolean = false;

    let toggleLabel: string;
    $: toggleLabel = valueShown ? "Hide Password" : "Show Password";
</script>

<Input
    {...$$restProps}
    disabled={disabled}
    type={valueShown ? "text" : "password"}
    bind:value
    on:blur
    on:change
    on:click
    on:contextmenu
    on:focus
    on:keydown
    on:keypress
    on:keyup
    on:mouseover
    on:mouseenter
    on:mouseleave
    on:paste
    on:input
>
    <button
        slot="right"
        on:click={() => valueShown = !valueShown}
        class="ml-auto rounded-lg p-2 -mr-1.5"
        disabled={disabled}
        aria-label={toggleLabel}
    >
        <SvgIcon icon={valueShown ? "VisibilityOff" : "VisibilityOn"} size="md" />
    </button>
</Input>
