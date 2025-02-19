import { getJsonComponent } from '../ui';

export const FmAlgo = getJsonComponent<{
    bgColor?: string;
    textColor?: string;
    borderColor?: string;
    encoderId?: number;
    audioPlugin: string;
    param: string;
    dataId?: string;
}>('FmAlgo');