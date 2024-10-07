<script lang="ts">
    import { Button, InfoPageLayout, SvgIcon } from "./ui";

    interface Props {
        onBack: () => void;
    }

    let { onBack }: Props = $props();
</script>

<InfoPageLayout>
    {#snippet title()}
        Setup Complete
    {/snippet}
    {#snippet body()}
        You can now close this page and unplug the calendar. If you want to change something later, plug in the calendar
        and press the RESET button on the back to enter setup mode again.
    {/snippet}
    <Button variant="outlined" class="w-1/2 min-w-fit" onclick={onBack}>
        <SvgIcon icon="ArrowBack" class="mr-2 -ml-2" size="md" />
        Back
    </Button>
</InfoPageLayout>

