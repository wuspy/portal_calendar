<script lang="ts">
    import Button from "./Button.svelte";
    import Card from "./Card.svelte";
    import SvgIcon from "./SvgIcon.svelte";
    import PageTitle from "./PageTitle.svelte";
    import type { Snippet } from "svelte";

    interface Props {
        title: string;
        backButtonLabel?: string;
        nextButtonLabel?: string;
        saving?: boolean;
        backDisabled?: boolean;
        nextDisabled?: boolean;
        onNext?: () => void;
        onBack?: () => void;
        children: Snippet;
    }

    let {
        title,
        backButtonLabel = "Back",
        nextButtonLabel = "Next",
        saving = false,
        backDisabled = false,
        nextDisabled = false,
        onNext,
        onBack,
        children,
    }: Props = $props();
</script>

<PageTitle>{title}</PageTitle>
<Card class="w-full my-2 p-4 sm:p-6 space-y-6 text-slate-900">
    {@render children()}
    <div class="w-full border-t pt-4 sm:pt-6 mt-4 sm:mt-6 flex space-x-4 sm:space-x-6">
        <Button variant="outlined" class="basis-1 grow min-w-fit" onclick={onBack} disabled={saving || backDisabled}>
            <SvgIcon icon="ArrowBack" class="-ml-2 mr-2" size="md" />
            {backButtonLabel}
        </Button>
        <Button loading={saving} class="basis-1 grow min-w-fit" onclick={onNext} disabled={nextDisabled}>
            {nextButtonLabel}
            <SvgIcon icon="ArrowForward" class="ml-2 -mr-2" size="md" />
        </Button>
    </div>
</Card>
