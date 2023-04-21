<script lang="ts">
    import { fade } from "svelte/transition";
    import { Button, Spinner, TransitionLayout } from "./ui";
    import SaveFailedModal from "./SaveFailedModal.svelte";
    import { loadLocales, loadPreferences, loadWifiInterfaceProperties, loadWifiStatus, wifiStatus } from "./store";
    import WeatherConfigPage from "./WeatherConfigPage.svelte";
    import DateTimeConfigPage from "./DateTimeConfigPage.svelte";
    import WifiSelectionPage from "./WifiSelectionPage.svelte";
    import WelcomePage from "./WelcomePage.svelte";
    import FinishPage from "./FinishPage.svelte";
    import { onMount, setContext } from "svelte";
    import WifiStatusPage from "./WifiStatusPage.svelte";
    import InfoPageLayout from "./ui/InfoPageLayout.svelte";

    let page: number = 0;

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
        page = $wifiStatus.connected ? 2 : 0;
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
                <WelcomePage on:next={onNext} />
            </TransitionLayout>
        {:else if page === 1}
            <TransitionLayout>
                <WifiSelectionPage on:next={onNext} />
            </TransitionLayout>
        {:else if page === 2}
            <TransitionLayout>
                <WifiStatusPage on:back={onBack} on:next={onNext} />
            </TransitionLayout>
        {:else if page === 3}
            <TransitionLayout>
                <DateTimeConfigPage on:next={onNext} on:back={onBack} />
            </TransitionLayout>
        {:else if page === 4}
            <TransitionLayout>
                <WeatherConfigPage on:next={onNext} on:back={onBack} />
            </TransitionLayout>
        {:else if page === 5}
            <TransitionLayout>
                <FinishPage on:back={onBack} />
            </TransitionLayout>
        {/if}
    {:catch}
        <TransitionLayout>
            <InfoPageLayout>
                <svelte:fragment slot="title">
                    Something went wrong
                </svelte:fragment>
                <svelte:fragment slot="body">
                    Failed to connect to calendar. Make sure that it's plugged in and
                    that you're connected to its WiFi network, then try again.
                </svelte:fragment>
                <Button class="w-1/2 min-w-fit" on:click={() => location.reload()}>Retry</Button>
            </InfoPageLayout>
        </TransitionLayout>
    {/await}
</main>

<SaveFailedModal />
