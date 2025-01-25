import { initializePlugin } from "../ui";

export type Props = {
    controller?: string;
    context?: string;
    key: string | number;
    action: string;
    action2?: string;
};

export function Keymap({ controller, context, key, action, action2 }: Props) {
    initializePlugin('Keymap', 'libzic_KeymapComponent.so');
    controller = controller || "Keyboard";
    if (context) {
        controller += `:${context}`;
    }
    if (typeof key === "string") {
        key = `'${key}'`;
    }
    return [
        {
            KEYMAP: `${controller} ${key} ${action}` + (action2 ? ` ${action2}` : ""),
        },
    ];
}
