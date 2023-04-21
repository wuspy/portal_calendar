<script lang="ts">
    import { createEventDispatcher } from "svelte";
    import type { WifiScanResponse } from "./model";
    import { connectToWifi, WifiConnectionError } from "./store";
    import { Button, Modal, Input, PasswordInput } from "./ui";

    const dispatch = createEventDispatcher<{
        close: void,
        connected: void,
    }>();

    export let network: WifiScanResponse | undefined;

    let ssid = network?.ssid ?? "";
    let password = "";

    let connectionError: WifiConnectionError | undefined = undefined;
    let connecting = false;

    async function connect() {
        connectionError = undefined;
        connecting = true;
        try {
            await connectToWifi(ssid, password);
            dispatch("connected");
        } catch (e) {
            connectionError = e as WifiConnectionError;
        } finally {
            connecting = false;
        }
    }

    let submitDisabled: boolean;
    $: submitDisabled = network
        ? password.length < 8
        : !ssid || (!!password && password.length < 8);

    let ssidError: string | undefined, passwordError: string | undefined;
    $: if (connectionError) {
        if (network) {
            passwordError = (<any>{
                [WifiConnectionError.AlreadyConnecting]: "Calendar is busy",
                [WifiConnectionError.NotFound]: "Network not found",
                [WifiConnectionError.Other]: "Check password and try again",
                [WifiConnectionError.RequestError]:
                    "Failed to connect to calendar. Make sure that it's plugged in"
                    + " and that you're connected to its WiFi network, then try again.",
            })[connectionError];
        } else {
            ssidError = (<any>{
                [WifiConnectionError.AlreadyConnecting]: "Calendar is busy",
                [WifiConnectionError.NotFound]: "Network not found",
                [WifiConnectionError.RequestError]:
                    "Failed to connect to calendar. Make sure that it's plugged in"
                    + " and that you're connected to its WiFi network, then try again.",
            })[connectionError];

            passwordError = (<any>{
                [WifiConnectionError.Other]: "Check password and try again",
            })[connectionError];
        }
    } else {
        ssidError = passwordError = undefined;
    }
</script>

<Modal title={network ? ssid : "Add Network"} permanent={connecting} size="xs" on:close>
    {#if !network}
        <Input
            label="Network name (SSID)"
            autofocus
            bind:value={ssid}
            status={{error: !!ssidError, message: ssidError }}
            required
            disabled={connecting}
        />
    {/if}
    <PasswordInput
        label="Password"
        autofocus={!!network}
        bind:value={password}
        status={{ error: !!passwordError, message: passwordError }}
        required={!!network}
        disabled={connecting}
        placeholder={network ? undefined : "(leave blank if no password)"}
    />
    <div class="flex flex-row space-x-4">
        <Button disabled={connecting} variant="outlined" class="grow" on:click={() => dispatch("close")}>Cancel</Button>
        <Button loading={connecting} disabled={submitDisabled} class="grow" on:click={connect}>Connect</Button>
    </div>
</Modal>
