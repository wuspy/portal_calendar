<script lang="ts">
    import { fade } from "svelte/transition";
    import { Button, InfoPageLayout, Spinner, TransitionLayout } from "./ui";
    import SaveFailedModal from "./SaveFailedModal.svelte";
    import { loadLocales, loadPreferences, loadWifiInterfaceProperties, loadWifiStatus, wifiStatus } from "./store";
    import WeatherConfigPage from "./WeatherConfigPage.svelte";
    import DateTimeConfigPage from "./DateTimeConfigPage.svelte";
    import WifiSelectionPage from "./WifiSelectionPage.svelte";
    import WelcomePage from "./WelcomePage.svelte";
    import FinishPage from "./FinishPage.svelte";
    import { onMount, setContext } from "svelte";
    import WifiStatusPage from "./WifiStatusPage.svelte";

    let page = $state(0);

    const transitionContext = setContext("transition", { direction: 1 });

    const onNext = () => {
        window.scrollTo(0, 0);
        transitionContext.direction = 1;
        ++page;
    };

    const onBack = () => {
        window.scrollTo(0, 0);
        transitionContext.direction = -1;
        --page;
    };

    const init = Promise.all([
        loadPreferences(),
        loadLocales(),
        loadWifiInterfaceProperties(),
        loadWifiStatus(),
    ]);

    onMount(async () => {
        await init;
        page = $wifiStatus?.connected ? 2 : 0;
    });
</script>

<main class="relative mx-auto max-w-xl">
    {#await init}
        <div
            transition:fade
            class="fixed z-50 top-0 left-0 w-screen h-screen flex items-center justify-center"
        >
            <Spinner width="128px" height="128px" />
        </div>
    {:then}
        {#if page === 0}
            <TransitionLayout>
                <WelcomePage {onNext} />
            </TransitionLayout>
        {:else if page === 1}
            <TransitionLayout>
                <WifiSelectionPage {onNext} />
            </TransitionLayout>
        {:else if page === 2}
            <TransitionLayout>
                <WifiStatusPage {onBack} {onNext} />
            </TransitionLayout>
        {:else if page === 3}
            <TransitionLayout>
                <DateTimeConfigPage {onNext} {onBack} />
            </TransitionLayout>
        {:else if page === 4}
            <TransitionLayout>
                <WeatherConfigPage {onNext} {onBack} />
            </TransitionLayout>
        {:else if page === 5}
            <TransitionLayout>
                <FinishPage {onBack} />
            </TransitionLayout>
        {/if}
    {:catch}
        <TransitionLayout>
            <InfoPageLayout>
                {#snippet title()}
                    Something went wrong
                {/snippet}
                {#snippet body()}
                    {#if $wifiStatus?.ssid}
                        Failed to connect to calendar. Make sure the calendar is plugged in, and that
                        you're connected either to its Wi-Fi network or the network '{$wifiStatus.ssid}',
                        then try again.
                    {:else}
                        Failed to connect to calendar. Make sure the calendar is plugged in and that
                        you're connected to its Wi-Fi network, then try again.
                    {/if}
                    <br/>
                    If this problem persists, open an issue at
                    <a href="https://github.com/wuspy/portal_calendar">https://github.com/wuspy/portal_calendar</a>.
                {/snippet}
                <Button class="w-1/2 min-w-fit" onclick={() => location.reload()}>Retry</Button>
            </InfoPageLayout>
        </TransitionLayout>
    {/await}
</main>

<SaveFailedModal />
