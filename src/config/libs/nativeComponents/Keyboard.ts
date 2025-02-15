import { getComponent, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
};

export function Keyboard({ bounds, items, ...props }: Props) {
    initializePlugin('Keyboard', 'libzic_KeyboardComponent.so');
    return getComponent('Keyboard', bounds, [props]);
}
