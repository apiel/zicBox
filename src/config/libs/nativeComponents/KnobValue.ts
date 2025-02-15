import { getJsonComponent } from '../ui';

export const KnobValue = getJsonComponent<{
    audioPlugin: string;
    param: string;
    encoderId?: number;
    type?: string;
    label?: string;
    color?: string;
    bgColor?: string;
    textColor?: string;
    floatPrecision?: number;
    hideValue?: boolean;
    hideUnit?: boolean;
    unitSize?: number;
    valueSize?: number;
    titleSize?: number;
    valueReplaceTitle?: boolean;
}>('KnobValue', 'libzic_KnobValueComponent.so');
