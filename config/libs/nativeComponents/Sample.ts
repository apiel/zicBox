import { getJsonComponent } from '../ui';

export const Sample = getJsonComponent<{
    audioPlugin: string;
    bgColor?: string;
    overlayColor?: string;
    overlayEdgeColor?: string;
    loopStartColor?: string;
    loopEndColor?: string;
    sampleColor?: string;
    valueKeys?: {
        browser?: string;
        start?: string;
        end?: string;
        loopPosition?: string;
        loopLength?: string;
    };
}>('Sample');
