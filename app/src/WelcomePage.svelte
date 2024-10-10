<script lang="ts">
    import { createEventDispatcher } from "svelte";
    import { Button, InfoPageLayout, SvgIcon } from "./ui";

    const dispatch = createEventDispatcher<{ next: void }>();
</script>

<InfoPageLayout>
    <svelte:fragment slot="title">
        Welcome!
    </svelte:fragment>
    <svelte:fragment slot="body">
        This is the setup page for your Portal calendar. To get started, you'll need to choose a Wi-Fi network
        that it can connect to.
    </svelte:fragment>
    <Button class="w-1/2 min-w-fit" on:click={() => dispatch("next")}>
        Get started
        <SvgIcon icon="ArrowForward" class="ml-2 -mr-2" size="md" />
    </Button>
</InfoPageLayout>
