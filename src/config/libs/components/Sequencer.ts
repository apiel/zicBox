import { getComponent, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    // data: string;
};

export function Sequencer({ position, track, ...props }: Props) {
    initializePlugin('Sequencer', 'libzic_SequencerComponent.so');
    return getComponent('Sequencer', position, [{ track }, props]);
}
