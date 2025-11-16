import { getJsonComponent } from '../ui';

export const GraphValue = getJsonComponent<{
    outline?: boolean;
    fillColor?: string;
    filled?: boolean;
    outlineColor?: string;
    bgColor?: string;
    audioPlugin: string;
    param: string;
    inverted?: boolean;
    extendEncoderIdArea?: number;
}>('GraphValue');
