import { getComponent } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
};

export function Keyboard({ position, items, ...props }: Props) {
    return getComponent('Keyboard', position, [props]);
}
