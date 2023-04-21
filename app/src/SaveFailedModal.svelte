<script context="module" lang="ts">
	import { Button, Modal } from "./ui";
    import { writable } from "svelte/store";

	const open = writable(false);
	export const openSaveFailedModal = () => open.set(true);
</script>

{#if $open}
	<Modal title="Error" size="xs">
		<p class="text-md">
			Failed to save settings. Make sure the calendar is plugged in and that
			you're connected to its WiFi network, then try again.
		</p>
		<div class="flex flex-row">
			<span class="grow-[2]" />
			<Button class="grow" on:click={() => open.set(false)}>Ok</Button>
		</div>
	</Modal>
{/if}
