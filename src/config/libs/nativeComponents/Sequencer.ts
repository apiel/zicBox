import { getJsonComponent } from '../ui';

export const Sequencer = getJsonComponent<{
    bgColor?: string;
    blackKeyColor?: string;
    whiteKeyColor?: string;
}>('Sequencer', 'libzic_SequencerComponent.so');
