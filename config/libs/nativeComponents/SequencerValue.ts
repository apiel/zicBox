import { getJsonComponent } from '../ui';

export const SequencerValue = getJsonComponent<{
    audioPlugin: string;
    bgColor?: string;
    stepsDataId?: string;
    maxSteps?: number;
    labelColor?: string;
    valueColor?: string;
    unitColor?: string;
    contextId?: number;
    valueSize?: number;
    labelSize?: number;
    fontValue?: string;
    fontLabel?: string;
    encoderId?: number;
}>('SequencerValue');
