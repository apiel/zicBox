import { getJsonComponent } from '../ui';

export const SequencerCard = getJsonComponent<{
    audioPlugin: string;
    bgColor?: string;
    stepsDataId?: string;
    stepPerRow?: number;
    maxSteps?: number;
    activeStepColor?: string;
    stepBackground?: string;
}>('SequencerCard');
