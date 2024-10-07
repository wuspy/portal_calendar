<script lang="ts">
    import classNames from "classnames";
    import type { Snippet } from "svelte";
    import type { HTMLAttributes } from "svelte/elements";

    interface Props extends HTMLAttributes<HTMLHeadingElement> {
        children: Snippet;
    }

    let { children, ...props }: Props = $props();
</script>

<h1 {...props} class={classNames("py-2 mx-1 text-2xl font-medium text-slate-900 tracking-tight", props.class)}>
    {@render children()}
</h1>
