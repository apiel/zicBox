import { getJsonComponent } from '../ui';

export const SequencerValue = getJsonComponent<{
    audioPlugin: string;
    bgColor?: string;
    stepsDataId?: string;
    maxSteps?: number;
    labelColor?: string;
    valueColor?: string;
    barColor?: string;
    contextId?: number;
    valueSize?: number;
    labelSize?: number;
    fontValue?: string;
    fontLabel?: string;
    encoderId?: number;
    type: "STEP_SELECTION" | "STEP_TOGGLE" | "STEP_NOTE" | "STEP_CONDITION" | "STEP_VELOCITY";
}>('SequencerValue');
