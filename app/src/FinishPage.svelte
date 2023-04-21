<script lang="ts">
    import { createEventDispatcher } from "svelte";
    import { Button, InfoPageLayout, SvgIcon } from "./ui";

    const dispatch = createEventDispatcher<{ back: void }>();
</script>

<InfoPageLayout>
    <svelte:fragment slot="title">
        Setup Complete
    </svelte:fragment>
    <svelte:fragment slot="body">
        You can now close this page and unplug the calendar. If you want to change something later, plug in the calendar
        and press the RESET button on the back to enter setup mode again.
    </svelte:fragment>
    <Button variant="outlined" class="w-1/2 min-w-fit" on:click={() => dispatch("back")}>
        <SvgIcon icon="ArrowBack" class="mr-2 -ml-2" size="md" />
        Back
    </Button>
</InfoPageLayout>

