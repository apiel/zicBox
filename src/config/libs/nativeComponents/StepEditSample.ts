import { getJsonComponent } from '../ui';

export const StepEditSample = getJsonComponent<{
    audioPlugin: string;
    stepIndex: number;
    bgColor?: string;
    textColor?: string;
    playingColor?: string;
    selectedColor?: string;
}>('StepEditSample');
