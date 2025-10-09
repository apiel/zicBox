import { getJsonComponent } from '../ui';

export const Wifi = getJsonComponent<{
    bgColor?: string;
    textColor?: string;
    foregroundColor?: string;
    activeColor?: string;
    fontSize?: number;
    font?: string;

    ssidEncoderId?: number;
    passwordEncoderId?: number;
    cursorEncoderId?: number;
    contextId?: number;
    default?: string;
}>('Wifi');
