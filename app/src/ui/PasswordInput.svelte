<script lang="ts">
    import type { ComponentProps } from "svelte";
    import Input from "./Input.svelte";
    import SvgIcon from "./SvgIcon.svelte";

    interface Props extends ComponentProps<Input> {
        valueShown?: boolean;
    }

    let { valueShown = $bindable(false), value = $bindable(""), ...props }: Props = $props();

    let toggleLabel = $derived(valueShown ? "Hide Password" : "Show Password");
</script>

<Input
    {...props}
    {value}
    type={valueShown ? "text" : "password"}
>
    {#snippet right()}
        <button
            onclick={() => valueShown = !valueShown}
            class="ml-auto rounded-lg p-2 -mr-1.5"
            disabled={props.disabled}
            aria-label={toggleLabel}
        >
            <SvgIcon icon={valueShown ? "VisibilityOff" : "VisibilityOn"} size="md" />
        </button>
    {/snippet}
</Input>
