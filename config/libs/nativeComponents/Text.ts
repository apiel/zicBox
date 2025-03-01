import { getJsonComponent } from '../ui';

export const Text = getJsonComponent<{
    text: string;
    fontSize?: number;
    fontHeight?: number;
    color?: string;
    bgColor?: string;
    centered?: boolean;
    right?: boolean;
    font?: string;
}>('Text');
