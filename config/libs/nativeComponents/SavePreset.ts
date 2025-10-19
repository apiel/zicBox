import { getJsonComponent } from '../ui';

export const SavePreset = getJsonComponent<{
    audioPlugin: string;
    folder?: string;
    redirectView?: string;
    bgColor?: string;
    textColor?: string;
    foregroundColor?: string;
    fontSize?: number;
    font?: string;
    valueEncoderId?: number;
    cursorEncoderId?: number;
}>('SavePreset');
