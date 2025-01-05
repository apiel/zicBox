import { getComponent } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
};

export function Text({ position, track, ...props }: Props) {
    return getComponent('Text', position, [{ track }, props]);
}
