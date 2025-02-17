import { getJsonComponent } from '../ui';

export const Sequencer = getJsonComponent<{
    audioPlugin: string;
    bgColor?: string;
    blackKeyColor?: string;
    whiteKeyColor?: string;
    // TODO add the rest of params
}>('Sequencer', 'libzic_SequencerComponent.so');
