import { getJsonComponent } from '../ui';

export const SavePreset = getJsonComponent<{
    bgColor?: string;
    textColor?: string;
    foregroundColor?: string;
    fontSize?: number;
    font?: string;
    valueEncoderId?: number;
    cursorEncoderId?: number;
}>('SavePreset');
