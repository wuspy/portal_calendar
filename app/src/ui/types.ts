export declare const xs = "xs"
export declare const sm = "sm"
export declare const md = "md"
export declare const lg = "lg"
export declare const xl = "xl"
export declare type SizeType = typeof xs | typeof sm | typeof md | typeof lg | typeof xl;
export declare type FormSizeType = typeof sm | typeof md | typeof lg;

export type InputStatus = { error?: boolean, validating?: boolean, success?: boolean, message?: string };
export type InputValidator = (value: any) => Promise<boolean | [boolean, string]>;
export type InputType = "color" | "date" | "datetime-local" | "email" | "file" | "hidden" | "image" | "month" | "number" | "password" | "reset" | "submit" | "tel" | "text" | "time" | "url" | "week" | "search";
export type ButtonType = "button" | "submit" | "reset";
export type ButtonVariant = "text" | "contained" | "outlined";
export type UiColor = "base" | "green" | "red" | "yellow";

export type SelectOptionType = {
	name: string | number;
	value: string | number;
};
