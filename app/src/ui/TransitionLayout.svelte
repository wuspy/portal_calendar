<script lang="ts">
    import { getContext, type Snippet } from "svelte";
    import type { HTMLAttributes } from "svelte/elements";
    import { fly } from "svelte/transition";

    let { children, ...props }: HTMLAttributes<HTMLDivElement> & { children: Snippet } = $props();

    const transition = getContext<{ direction: number }>("transition");
</script>

<div
    {...props}
    class="absolute w-full p-4 {props.class}"
    in:fly={{ x: 200 * transition.direction, duration: 500 }}
    out:fly={{ x: -200 * transition.direction, duration: 500 }}
>
    {@render children()}
</div>
