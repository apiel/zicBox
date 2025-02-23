import { getJsonComponent } from '../ui';

export const DrumEnvelop = getJsonComponent<{
    audioPlugin: string;
    envelopDataId: string;
    outline?: boolean;
    fillColor?: string;
    filled?: boolean;
    outlineColor?: string;
    bgColor?: string;
    renderValuesOnTop?: boolean;
    textColor?: string;
    cursorColor?: string;
    encoderTime?: number;
    encoderModulation?: number;
    encoderPhase?: number;
}>('DrumEnvelop');