<script lang="ts">
    import IconButton from "./IconButton.svelte";
    export let name: string = "Close";
</script>

<IconButton
    icon="Close"
    on:click
    {name}
    {...$$restProps}
    class="ml-auto {$$props.class}"
/>
