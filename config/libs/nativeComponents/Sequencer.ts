import { getJsonComponent } from '../ui';

export const Sequencer = getJsonComponent<{
    audioPlugin: string;
    bgColor?: string;
    blackKeyColor?: string;
    whiteKeyColor?: string;
    selectedColor?: string;
    stepColor?: string;
    maxStepLen?: number;
    // TODO add the rest of params
}>('Sequencer');
