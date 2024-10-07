<script lang="ts">
    import { setContext, type ComponentProps } from "svelte";
    import classNames from "classnames";
    import Frame from "./utils/Frame.svelte";

    interface Props extends Omit<ComponentProps<Frame>, "rounded" | "border" | "shadow"> {
        active?: boolean;
    }

    let { active = false, ...props }: Props = $props();

    $effect(() => {
        setContext("active", active)
    });
</script>

<Frame
    tag={active ? "div" : "ul"}
    {...props}
    rounded
    border
    shadow
    class={classNames("divide-y divide-gray-200", props.class)}
    role="list"
/>
