import { getJsonComponent } from '../ui';

export const Keyboard = getJsonComponent<{
    redirectView?: string;
    audioPlugin?: string;
    dataId?: string;
    bgColor?: string;
    textColor?: string;
    selectionColor?: string;
    itemBackground?: string;
}>('Keyboard');
