<script lang="ts">
    import type { ComponentProps } from "svelte";
    import IconButton from "./IconButton.svelte";

    interface Props extends Omit<ComponentProps<IconButton>, "icon"> {
        name?: string;
    }

    let { name = "Close", ...props }: Props = $props();
</script>

<IconButton
    icon="Close"
    {name}
    {...props}
    class="ml-auto {props.class}"
/>
