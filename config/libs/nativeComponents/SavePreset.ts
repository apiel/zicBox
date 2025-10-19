import { getJsonComponent } from '../ui';

export const SavePreset = getJsonComponent<{
    audioPlugin: string;
    bgColor?: string;
    textColor?: string;
    foregroundColor?: string;
    fontSize?: number;
    font?: string;
    valueEncoderId?: number;
    cursorEncoderId?: number;
}>('SavePreset');
