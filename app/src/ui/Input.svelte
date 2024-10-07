<script context="module" lang="ts">
    import type { SizeType, FormSizeType, InputStatus } from "./types";
    export function clampSize(s: SizeType) {
        return s && s === "xs" ? "sm" : s === "xl" ? "lg" : s;
    }
</script>

<script lang="ts">
    import Wrapper from "./utils/Wrapper.svelte";
    import classNames from "classnames";
    import { getContext, onMount, type Snippet } from "svelte";
    import type { InputType, UiColor, InputValidator } from "./types";
    import Label from "./Label.svelte";
    import Helper from "./Helper.svelte";
    import type { HTMLInputAttributes } from "svelte/elements";

    interface Props extends Omit<HTMLInputAttributes, "size" | "children"> {
        type?: InputType;
        size?: FormSizeType;
        label?: string;
        help?: string;
        status?: InputStatus;
        validator?: InputValidator;
        validateOnMount?: boolean;
        left?: Snippet;
        right?: Snippet;
    }

    let {
        type = "text",
        value = $bindable(""),
        size = undefined,
        label = "",
        help = "",
        status = $bindable({}),
        validator,
        validateOnMount = false,
        autofocus,
        disabled,
        required,
        left,
        right,
        ...inputProps
    }: Props = $props();

    let inputElement: HTMLInputElement;

    const colorClasses: Record<UiColor, string> = {
        base: "bg-gray-50 border-gray-300 focus:border-gray-500 focus:ring-gray-500",
        green: "bg-green-50 border-green-500 focus:ring-green-500 focus:border-green-500",
        red: "bg-red-50 border-red-500 focus:ring-red-500 focus:border-red-500",
        yellow: "bg-yellow-50 border-yellow-500 focus:ring-yellow-500 focus:border-yellow-500",
    };

    const textColorClasses: Record<UiColor, string> = {
        base: "text-gray-900 placeholder-gray-400",
        green: "text-green-900 placeholder-green-700",
        red: "text-red-900 placeholder-red-700",
        yellow: "text-yellow-900 placeholder-yellow-700",
    };

    const disabledClass = "disabled:cursor-not-allowed disabled:opacity-50";

    let group: { size: SizeType } = getContext("group");

    // you need to this to avoid 2-way binding
    const setType = (node: HTMLInputElement, _type: string) => {
        node.type = _type;
        return {
            update(_type: string) {
                node.type = _type;
            },
        };
    };

    const textSizes = { sm: "sm:text-xs", md: "text-sm", lg: "sm:text-base" };
    const leftPadding = { sm: "pl-9", md: "pl-10", lg: "pl-11" };
    const rightPadding = { sm: "pr-9", md: "pr-10", lg: "pr-11" };
    const inputPadding = { sm: "p-2", md: "p-2.5", lg: "p-4" };

    let color: UiColor = $derived(status.validating ? "yellow" : status.error ? "red" : status.success ? "green" : "base");
    let message = $derived(status.validating || status.error || status.success ? status.message ?? help : help);
    let _size = $derived(size || clampSize(group?.size) || "md");
    let inputClass = $derived(classNames(
        "block w-full",
        disabledClass,
        left && leftPadding[_size],
        right && rightPadding[_size],
        colorClasses[color],
        textColorClasses[color],
        inputPadding[_size],
        textSizes[_size],
        group || "rounded-lg",
        group && "first:rounded-l-lg last:rounded-r-lg",
        group && "border-l-0 first:border-l last:border-r",
        inputProps.class
    ));
    let floatClass = "flex absolute inset-y-0 items-center text-gray-500";

    let validatorCount = 0;

    const onChange = async (value: string | number) => {
        if (required && !value) {
            status = { error: true, message: "Required" };
        } else if (validator) {
            status = { validating: true, message: "Checking..." };
            try {
                let thisValidatorCount = ++validatorCount;
                const result = await validator(value);
                if (thisValidatorCount !== validatorCount) {
                    return;
                }
                const [valid, message] = Array.isArray(result) ? result : [result, undefined];
                status = { success: valid && !!message, error: !valid, message };
            } catch (e: any) {
                status = { success: false, message: "Error validating field" };
            }
        } else {
            status = {};
        }
    };

    export const revalidate = () => {
        onChange(value);
    };

    onMount(() => {
        if (autofocus) {
            inputElement.focus();
        }
        if ((validator || required) && validateOnMount) {
            onChange(value);
        }
    });
</script>

<Label class="space-y-2" {disabled} show={!!label || !!message}>
    {#if label}<span>{label}</span>{/if}
    <Wrapper class="relative w-full" show={!!(left || right)}>
        {#if left}
            <div class={classNames(floatClass, disabledClass, textColorClasses[color], "left-0 pl-2.5 pointer-events-none")}>{@render left()}</div>
        {/if}
        <input
            {...inputProps}
            {disabled}
            {required}
            class={inputClass}
            bind:value
            bind:this={inputElement}
            use:setType={type}
            onchange={e => {
                inputProps.onchange && inputProps.onchange(e);
                onChange(e.currentTarget.value);
            }}
        />
        {#if right}
            <div class={classNames(floatClass, disabledClass, textColorClasses[color], "right-0 pr-2.5")}>{@render right()}</div>
        {/if}
    </Wrapper>
    {#if message}<Helper {color}>{message}</Helper>{/if}
</Label>
