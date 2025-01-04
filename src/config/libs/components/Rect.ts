import { getComponent } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
};

export function Rect({ position, track, ...props }: Props) {
    return getComponent('Rect', position, [{ track }, props]);
}
