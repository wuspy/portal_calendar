<script lang="ts">
    import { Button, InfoPageLayout, SvgIcon } from "./ui";

    interface Props {
        onNext: () => void;
    }

    let { onNext }: Props = $props();
</script>

<InfoPageLayout>
    {#snippet title()}
        Welcome!
    {/snippet}
    {#snippet body()}
        This is the setup page for your Portal calendar. To get started, you'll need choose a Wi-Fi network
        that it can connect to.
    {/snippet}
    <Button class="w-1/2 min-w-fit" onclick={onNext}>
        Get started
        <SvgIcon icon="ArrowForward" class="ml-2 -mr-2" size="md" />
    </Button>
</InfoPageLayout>
