import { getJsonComponent } from '../ui';

export const Sequencer = getJsonComponent<{
    bgColor?: string;
}>('Sequencer', 'libzic_SequencerComponent.so');
