import { getJsonComponent } from '../ui';

export const Clip = getJsonComponent<{
    bgColor?: string;
    textColor?: string;
    fontSize?: number;
    font?: string;
    smallFontSize?: number;
    smallFont?: string;
    sequencerPlugin?: string;
    serializerPlugin?: string;
}>('Clip');
