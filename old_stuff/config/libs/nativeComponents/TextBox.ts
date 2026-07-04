import { getJsonComponent } from '../ui';

export const TextBox = getJsonComponent<{
    text: string;
    fontSize?: number;
    fontHeight?: number;
    color?: string;
    color2?: string;
    bgColor?: string;
    font?: string;
}>('TextBox');
