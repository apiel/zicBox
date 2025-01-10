import { getComponent } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
};

export function Tape({ position, track, ...props }: Props) {
    return getComponent('Tape', position, [{ track }, props]);
}
