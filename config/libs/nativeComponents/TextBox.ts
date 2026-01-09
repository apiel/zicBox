import { getJsonComponent } from '../ui';

export const TextBox = getJsonComponent<{
    text: string;
    fontSize?: number;
    fontHeight?: number;
    color?: string;
    bgColor?: string;
    font?: string;
}>('TextBox');
