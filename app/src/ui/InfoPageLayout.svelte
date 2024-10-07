<script lang="ts">
    import type { Snippet } from "svelte";

    interface Props {
        title: Snippet;
        body: Snippet;
        children: Snippet;
    }

    let { title, body, children }: Props = $props();

</script>

<div class="flex flex-col h-screen items-center justify-center space-y-6 mx-2 -my-20">
    <h1 class="text-center text-slate-900 font-bold text-4xl sm:text-5xl tracking-tight">
        {@render title()}
    </h1>
    <p class="text-center text-slate-600 text-lg tracking-normal">
        {@render body()}
    </p>
    {@render children()}
</div>
