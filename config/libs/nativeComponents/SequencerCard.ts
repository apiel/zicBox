import { getJsonComponent } from '../ui';

export const SequencerCard = getJsonComponent<{
    audioPlugin: string;
    bgColor?: string;
    stepsDataId?: string;
    stepPerRow?: number;
    maxSteps?: number;
    activeStepColor?: string;
    stepBackground?: string;
    inactiveStepColor?: string;
    stepSelectedColor?: string;
    rowsSelectionColor?: string;
    rowsSelection?: number;
    contextId?: number;
}>('SequencerCard');
