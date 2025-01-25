import { getComponent, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
};

export function Tape({ position, track, ...props }: Props) {
    initializePlugin('Tape', 'libzic_TapeComponent.so');
    return getComponent('Tape', position, [{ track }, props]);
}
