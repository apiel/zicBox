import { getJsonComponent } from '../ui';

export const StringVal = getJsonComponent<{
    audioPlugin: string;
    param: string;
    labelSize?: number;
    fontLabel?: string;
    bgColor?: string;
    labelColor?: string;
}>('StringVal');
