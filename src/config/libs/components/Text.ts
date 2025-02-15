import { getJsonComponent } from '../ui';

export const Text = getJsonComponent<{
    text: string;
    fontSize?: number;
    fontHeight?: number;
    color?: string;
    bgColor?: string;
    centered?: boolean;
    font?: string;
}>('Text', 'libzic_TextComponent.so');
