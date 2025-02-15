import { getComponent, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
};

export function Tape({ bounds, track, ...props }: Props) {
    initializePlugin('Tape', 'libzic_TapeComponent.so');
    return getComponent('Tape', bounds, [{ track }, props]);
}
