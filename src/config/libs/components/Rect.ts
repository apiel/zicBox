import { getComponent, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
};

export function Rect({ position, track, ...props }: Props) {
    initializePlugin('Rect', 'libzic_RectComponent.so');
    return getComponent('Rect', position, [{ track }, props]);
}
