<script context="module" lang="ts">
    import { wifiStatus } from "./store";
    import { Button, Modal } from "./ui";
    import { writable } from "svelte/store";

    const open = writable(false);
    export const openSaveFailedModal = () => open.set(true);
</script>

{#if $open}
    <Modal title="Error" size="xs">
        <p class="text-md">
            {#if $wifiStatus.ssid}
                Failed to save settings. Make sure the calendar is plugged in, and that
                you're connected either to its Wi-Fi network or the network '{$wifiStatus.ssid}',
                then try again.
            {:else}
                Failed to save settings. Make sure the calendar is plugged in and that
                you're connected to its Wi-Fi network, then try again.
            {/if}
        </p>
        <p class="text-md">
            If this problem persists, open an issue at
            <a href="https://github.com/wuspy/portal_calendar">https://github.com/wuspy/portal_calendar</a>.
        </p>
        <div class="flex flex-row">
            <span class="grow-[2]" />
            <Button class="grow" on:click={() => open.set(false)}>Ok</Button>
        </div>
    </Modal>
{/if}
