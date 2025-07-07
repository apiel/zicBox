import { getJsonComponent } from '../ui';

export const MacroEnvelop = getJsonComponent<{
    audioPlugin: string;
    envelopDataId: string;
    outline?: boolean;
    fillColor?: string;
    filled?: boolean;
    outlineColor?: string;
    bgColor?: string;
    textColor?: string;
    cursorColor?: string;
    encoders?: [number, number, number, number];
    title?: string;
    titleColor?: string;
}>('MacroEnvelop');