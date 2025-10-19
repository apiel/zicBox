import { getJsonComponent } from '../ui';

export const Preset = getJsonComponent<{
    audioPlugin: string;
    folder?: string;
    bgColor?: string;
    textColor?: string;
    foregroundColor?: string;
    fontSize?: number;
    font?: string;
    encoderId?: number;
}>('Preset');
