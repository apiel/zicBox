import { getJsonComponent } from '../ui';

export const Keyboard2 = getJsonComponent<{
    redirectView?: string;
    audioPlugin?: string;
    dataId?: string;
    bgColor?: string;
    textColor?: string;
    selectionColor?: string;
    itemBackground?: string;
    font?: string;
    shiftContextId?: number;
}>('Keyboard2');
