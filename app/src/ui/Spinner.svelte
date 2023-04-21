<script context="module" lang="ts">
    import classNames from "classnames";

    const path = "M10 77 L126 55 L66 15 A126 126 0 0 0 10 77";
    const rotations = [45, 90, 135, 180, 225, 270, 315];
</script>

<svg fill="currentColor" {...$$props} class={classNames("spinner", $$props.class)} viewBox="0 0 252 252">
    <path d={path} />
    {#each rotations as rotation (rotation)}
        <path d={path} transform="rotate({rotation}, 126, 126)" />
    {/each}
</svg>

<style>
    .spinner {
        animation: spin 2s linear infinite;
    }

    @keyframes spin {
        from {
            transform: rotate(360deg);
        }
        to {
            transform: rotate(0deg);
        }
    }
</style>
