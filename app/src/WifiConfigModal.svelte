<script lang="ts">
    import type { WifiScanResponse } from "./model";
    import { connectToWifi, WifiConnectionError } from "./store";
    import { Button, Modal, Input, PasswordInput } from "./ui";

    interface Props {
        onClose: () => void;
        onConnected: () => void;
        network?: WifiScanResponse;
    }

    let {
        onClose,
        onConnected,
        network
    }: Props = $props();

    let ssid = $state(network?.ssid ?? "");
    let password = $state("");

    let connectionError: WifiConnectionError | undefined = $state();
    let connecting = $state(false);

    async function connect() {
        connectionError = undefined;
        connecting = true;
        try {
            await (network?.open ? connectToWifi(ssid.trim()) : connectToWifi(ssid.trim(), password));
            onConnected();
        } catch (e) {
            connectionError = e as WifiConnectionError;
        } finally {
            connecting = false;
        }
    }

    let submitDisabled = $derived(
        network && !network.open
            ? password.length < 8
            : !ssid || (!!password && password.length < 8)
    );

    let ssidError = $derived.by(() => {
        if (connectionError && !network) {
            return ({
                [WifiConnectionError.AlreadyConnecting]: "Calendar is busy",
                [WifiConnectionError.NotFound]: "Network not found",
                [WifiConnectionError.RequestError]:
                    "Failed to connect to calendar. Make sure that it's plugged in"
                    + " and that you're connected to its Wi-Fi network, then try again.",
                [WifiConnectionError.Other]: undefined,
            })[connectionError];
        } else {
            return undefined;
        }
    });

    let passwordError = $derived.by(() => {
        if (connectionError) {
            if (network) {
                return ({
                    [WifiConnectionError.AlreadyConnecting]: "Calendar is busy",
                    [WifiConnectionError.NotFound]: "Failed to connect to this network, it may be out of range.",
                    [WifiConnectionError.Other]: network.open
                        ? "Failed to connect to this network"
                        : "Check password and try again",
                    [WifiConnectionError.RequestError]:
                        "Failed to connect to calendar. Make sure that it's plugged in"
                        + " and that you're connected to its Wi-Fi network, then try again.",
                })[connectionError];
            } else {
                return ({
                    [WifiConnectionError.Other]: "Check password and try again",
                    [WifiConnectionError.AlreadyConnecting]: undefined,
                    [WifiConnectionError.NotFound]: undefined,
                    [WifiConnectionError.RequestError]: undefined,
                })[connectionError];
            }
        } else {
            return undefined;
        }
    });
</script>

<Modal title={network ? ssid : "Add Network"} permanent={connecting} size="xs" on:close>
    {#if network?.open}
        <p class="text-md">
            This Wi-Fi network is insecure. Connect to it anyway?
        </p>
        {#if passwordError}
            <p class="text-md text-red-900">
                {passwordError}
            </p>
        {/if}
    {:else}
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
    {/if}
    <div class="flex flex-row space-x-4">
        <Button disabled={connecting} variant="outlined" class="grow" onclick={onClose}>Cancel</Button>
        <Button loading={connecting} disabled={submitDisabled} class="grow" onclick={connect}>Connect</Button>
    </div>
</Modal>
