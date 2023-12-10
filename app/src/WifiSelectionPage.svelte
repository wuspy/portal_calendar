<script lang="ts">
    import { fly } from "svelte/transition";
    import { Card, Button, IconButton, getWifiRssiIcon, PageTitle, ListGroup, ListGroupItem, Spinner, SvgIcon } from "./ui";
    import WifiConfigModal from "./WifiConfigModal.svelte";
    import type { WifiScanResponse } from "./model";
    import Api from "./api";
    import WifiInterfaceConfigModal from "./WifiInterfaceConfigModal.svelte";
    import { wifiStatus } from "./store";
    import { createEventDispatcher } from "svelte";
    import ConfirmForgetNetworkModal from "./ConfirmForgetNetworkModal.svelte";

    let propertiesModalOpen = false;
    let connectModalOpen = false;
    let confirmForgetNetworkModalOpen = false;
    let selectedNetwork: WifiScanResponse | undefined;

    const dispatch = createEventDispatcher<{ next: void }>();

    function onNetworkClick(network: WifiScanResponse) {
        if (network.ssid === $wifiStatus.ssid && $wifiStatus.connected) {
            // Already connected to this network
            dispatch("next");
        } else {
            selectedNetwork = network;
            connectModalOpen = true;
        }
    }

    function onAddNetworkClick() {
        selectedNetwork = undefined;
        connectModalOpen = true;
    }

    async function refreshNetworks() {
        try {
            const response = await Api.scanWifi();
            if (response.status !== 200) {
                return [];
            }
            let ssids = new Set<string>();
            return (await response.json())
                .filter(({ ssid }) => {
                    // ESP32 wifi returns all networks with unique BSSIDs, so there can be duplicate SSIDs.
                    // But we only want the one with the strongest signal to appear in the list.
                    if (ssids.has(ssid)) {
                        console.warn("Duplicate SSID:", ssid);
                        return false;
                    }
                    ssids.add(ssid);
                    return true;
                });
        } catch (e) {
            return [];
        }
    }

    let networks = refreshNetworks();
</script>

<div class="flex items-center">
    <PageTitle class="grow">Select Wi-Fi Network</PageTitle>
    {#await networks then}
        <IconButton icon="Refresh" name="Refresh" size="lg" on:click={() => networks = refreshNetworks()} />
    {/await}
</div>
{#await networks}
    <Card class="w-full my-2 p-4 text-base font-semibold">
        <div class="flex items-center space-x-4 text-slate-900">
            <Spinner width=24 height=24 />
            <span>Searching...</span>
        </div>
    </Card>
{:then networks}
    <div in:fly="{{y: -20, duration: 250}}">
        {#if networks.length}
            <ListGroup active class="w-full my-2">
                {#each networks as network (network.ssid)}
                    <ListGroupItem on:click={() => onNetworkClick(network)} class="text-base font-semibold gap-4">
                        <SvgIcon icon={getWifiRssiIcon(network.rssi)} size="lg" />
                        <div class="grow min-w-0 truncate">{network.ssid}</div>
                        {#if !network.open}
                            <SvgIcon icon="Lock" size="lg" title="Network is password protected" />
                        {/if}
                    </ListGroupItem>
                {/each}
            </ListGroup>
        {:else}
            <Card class="w-full my-2 p-4 text-base font-semibold text-slate-900">
                No Wi-Fi networks found.
            </Card>
        {/if}
    </div>
{/await}
<div class="my-3 space-y-3 flex flex-col items-start">
    {#if $wifiStatus.connected}
        <Button variant="text" on:click={() => dispatch("next")}>
            <SvgIcon icon="Undo" size="md" class="mr-4" />
            Keep using '{$wifiStatus.ssid}'
        </Button>
    {/if}
    {#if $wifiStatus.ssid}
        <Button variant="text" on:click={() => confirmForgetNetworkModalOpen = true}>
            <SvgIcon icon="Close" size="md" class="mr-4" />
            Forget network '{$wifiStatus.ssid}'
        </Button>
    {/if}
    <Button variant="text" on:click={onAddNetworkClick}>
        <SvgIcon icon="Add" size="md" class="mr-4" />
        <span>Add network manually</span>
    </Button>
    <Button variant="text" on:click={() => propertiesModalOpen = true}>
        <SvgIcon icon="Tune" size="md" class="mr-4" />
        <span>Interface properties</span>
    </Button>
</div>

{#if connectModalOpen}
    <WifiConfigModal
        network={selectedNetwork}
        on:close={() => connectModalOpen = false}
        on:connected={() => dispatch("next")}
    />
{/if}

{#if propertiesModalOpen}
    <WifiInterfaceConfigModal on:close={() => propertiesModalOpen = false}/>
{/if}

{#if confirmForgetNetworkModalOpen}
    <ConfirmForgetNetworkModal on:close={() => confirmForgetNetworkModalOpen = false}/>
{/if}
