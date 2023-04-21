<script lang="ts">
    import { getContext } from "svelte";
    import { fly } from "svelte/transition";

    const transition = getContext<{ direction: number }>("transition");
</script>

<div
    class="absolute w-full p-4"
    in:fly={{ x: 200 * transition.direction, duration: 500 }}
    out:fly={{ x: -200 * transition.direction, duration: 500 }}
>
    <slot />
</div>
