import { getJsonComponent } from '../ui';

export const GraphEncoder = getJsonComponent<{
    outline?: boolean;
    fillColor?: string;
    filled?: boolean;
    outlineColor?: string;
    textColor1?: string;
    textColor2?: string;
    bgColor?: string;
    audioPlugin: string;
    dataId: string;
    values?: string[];
    isArrayData?: boolean;
    renderValuesOnTop?: boolean;
    inverted?: boolean;
}>('GraphEncoder');
