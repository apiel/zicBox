
export type Props = {
    controller?: string;
    context?: string;
    key: string | number;
    action: string;
    action2?: string;
};

export function Keymap({ controller, context, key, action, action2 }: Props) {
    controller = controller || "Keyboard";
    if (context) {
        controller += `:${context}`;
    }
    if (typeof key === "string" && key.length === 1) {
        key = `'${key}'`;
    }
    return [
        {
            KEYMAP: `${controller} ${key} ${action}` + (action2 ? ` ${action2}` : ""),
        },
    ];
}
