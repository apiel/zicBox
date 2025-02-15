import { getComponent, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
};

export function Rect({ bounds, track, ...props }: Props) {
    initializePlugin('Rect', 'libzic_RectComponent.so');
    return getComponent('Rect', bounds, [{ track }, props]);
}
