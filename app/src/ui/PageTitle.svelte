<script lang="ts">
    import classNames from "classnames";
</script>

<h1 {...$$props} class={classNames("py-2 mx-1 text-2xl font-medium text-slate-900 tracking-tight", $$props.class)}>
    <slot />
</h1>
