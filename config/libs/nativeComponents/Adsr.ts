import { getJsonComponent } from '../ui';

export const Adsr = getJsonComponent<{
    outline?: boolean;
    fillColor?: string;
    filled?: boolean;
    outlineColor?: string;
    textColor1?: string;
    textColor2?: string;
    bgColor?: string;
    audioPlugin: string;
    encoders?: [number, number, number, number];
    values?: [string, string, string, string];
    renderValuesOnTop?: boolean;
}>('Adsr');