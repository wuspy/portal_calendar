<script lang="ts">
	import { SvgIcon, getWifiRssiIcon, WizardPageLayout } from "./ui";
    import { loadWifiStatus, wifiStatus } from "./store";
    import { onMount } from "svelte";

    let signalStrength: string;
    $: if ($wifiStatus.connected) {
        signalStrength = `${$wifiStatus.rssi} dBm `
        if ($wifiStatus.rssi > -56) {
            signalStrength += "(Excellent)";
        } else if ($wifiStatus.rssi > -67) {
            signalStrength += "(Good)";
        } else if ($wifiStatus.rssi > -78) {
            signalStrength += "(Fair)";
        } else if ($wifiStatus.rssi > -89) {
            signalStrength += "(Weak)";
        } else {
            signalStrength += "(Very Weak)";
        }
    }

    const statusLabelClass = "text-xs font-medium text-white px-1.5 py-0.5 rounded uppercase";

    onMount(() => {
        const t = setInterval(loadWifiStatus, 5000);
        return () => clearInterval(t);
    });
</script>

<WizardPageLayout title="Wifi Network" backButtonLabel="Change" on:next on:back nextDisabled={!$wifiStatus.connected}>
    <div class="w-full flex flex-col items-center">
        <SvgIcon icon={getWifiRssiIcon($wifiStatus.connected && $wifiStatus.rssi)} size="xl" />
        <h1 class="mt-2 mb-1 text-2xl font-medium break-all">{$wifiStatus.ssid}</h1>
        {#if $wifiStatus.connected}
            <div class="{statusLabelClass} bg-green-500">Connected</div>
        {:else}
            <div class="{statusLabelClass} bg-red-500">Not Connected</div>
        {/if}
    </div>
    {#if $wifiStatus.connected}
        <div class="flex flex-row items-center">
            <div class="space-y-1 flex flex-col items-center basis-1 grow min-w-fit">
                <div class="font-medium text-sm">IP address</div>
                <div class="font-normal text-sm text-gray-500">{$wifiStatus.ip}</div>
            </div>
            <div class="space-y-1 flex flex-col items-center basis-1 grow min-w-fit">
                <div class="font-medium text-sm">Signal strength</div>
                <div class="font-normal text-sm text-gray-500">{signalStrength}</div>
            </div>
        </div>
    {/if}
</WizardPageLayout>
