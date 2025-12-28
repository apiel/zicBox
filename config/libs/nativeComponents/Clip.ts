import { getJsonComponent } from '../ui';

export const Clip = getJsonComponent<{
    bgColor?: string;
    textColor?: string;
    fontSize?: number;
    font?: string;
}>('Clip');
