<script lang="ts">
    import { fade, fly } from "svelte/transition";
    import classNames from "classnames";
    import Frame from "./utils/Frame.svelte";
    import CloseButton from "./CloseButton.svelte";
    import focusTrap from "./utils/focusTrap";
    import type { SizeType } from "./types";
    import type { ComponentProps } from "svelte";

    interface Props extends ComponentProps<Frame> {
        title?: string;
        size?: SizeType;
        permanent?: boolean;
        onClose?: () => void;
    }

    let {
        title = "",
        size = "md",
        permanent = false,
        onClose = () => {},
        children,
        ...props
    }: Props = $props();

    function prepareFocus(node: HTMLElement) {
        const walker = document.createTreeWalker(node, NodeFilter.SHOW_ELEMENT);
        let n: Node | null;
        while ((n = walker.nextNode())) {
            if (n instanceof HTMLElement) {
                const el = n as HTMLElement;
                const [x, y] = isScrollable(el);
                if (x || y) el.tabIndex = 0;
            }
        }
        node.focus();
    }

    const sizes = {
        xs: "max-w-md",
        sm: "max-w-lg",
        md: "max-w-2xl",
        lg: "max-w-4xl",
        xl: "max-w-7xl",
    };

    let frameClass = $derived(classNames("relative flex flex-col mx-auto w-full max-h-full", sizes[size], props.class));
    const isScrollable = (e: HTMLElement): boolean[] => [
        e.scrollWidth > e.clientWidth &&
            ["scroll", "auto"].indexOf(getComputedStyle(e).overflowX) >= 0,
        e.scrollHeight > e.clientHeight &&
            ["scroll", "auto"].indexOf(getComputedStyle(e).overflowY) >= 0,
    ];
    function preventWheelDefault(e: Event) {
        e.stopPropagation();
        // @ts-ignore
        const [x, y] = isScrollable(this);
        return x || y || e.preventDefault();
    }
    function handleKeys(e: KeyboardEvent) {
        e.stopPropagation();
        if (e.key === "Escape" && !permanent) {
            e.preventDefault();
            onClose();
        }
    }
</script>

<!-- backdrop -->
<div transition:fade="{{duration: 200}}" class="fixed inset-0 z-40 bg-gray-900 bg-opacity-50"></div>
<!-- dialog -->
<!-- svelte-ignore a11y_no_noninteractive_element_interactions -->
<div
    transition:fly="{{y: -20, duration: 200}}"
    onkeydown={handleKeys}
    onwheel={e => e.preventDefault()}
    use:prepareFocus
    use:focusTrap
    class="fixed top-0 left-0 right-0 h-modal md:inset-0 md:h-full z-50 w-full p-4 flex justify-center items-start"
    tabindex="-1"
    aria-modal="true"
    role="dialog"
>
    <Frame rounded shadow {...props} class={frameClass}>
        {#if !permanent}
            <CloseButton
                name="Close dialog"
                class="absolute top-3 right-3"
                onclick={onClose}
            />
        {/if}
        <!-- svelte-ignore a11y_no_static_element_interactions -->
        <div
            id="modal"
            class="p-6 space-y-6 flex-1 overflow-y-auto overscroll-contain"
            onkeydown={handleKeys}
            onwheel={preventWheelDefault}
        >
            {#if title}
                <h2 class="text-xl font-medium text-gray-900 mr-6">{title}</h2>
            {/if}
            {@render children()}
        </div>
    </Frame>
</div>
