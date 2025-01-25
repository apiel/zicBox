import { getComponent, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
};

export function Keyboard({ position, items, ...props }: Props) {
    initializePlugin('Keyboard', 'libzic_KeyboardComponent.so');
    return getComponent('Keyboard', position, [props]);
}
