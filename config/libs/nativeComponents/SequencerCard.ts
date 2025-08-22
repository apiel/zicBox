import { getJsonComponent } from '../ui';

export const SequencerCard = getJsonComponent<{
    audioPlugin: string;
    bgColor?: string;
    stepsDataId?: string;
    stepPerRow?: number;
    maxSteps?: number;
    activeStepColor?: string;
    stepLengthColor?: string;
    stepBackground?: string;
    inactiveStepColor?: string;
    stepSelectedColor?: string;
    rowsSelectionColor?: string;
    rowsSelection?: number;
    contextId?: number;
    gridKeys?: (number | string)[];
    encoderId?: number;
}>('SequencerCard');
