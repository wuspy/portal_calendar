<script lang="ts">
    import { setContext } from "svelte";
    import classNames from "classnames";
    import Frame from "./utils/Frame.svelte";

    export let active: boolean = false;
    $: setContext("active", active);
</script>

<Frame
    tag={active ? "div" : "ul"}
    {...$$restProps}
    rounded
    border
    shadow
    class={classNames("divide-y divide-gray-200", $$props.class)}
    role="list"
>
    <slot />
</Frame>
