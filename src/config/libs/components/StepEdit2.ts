import { getComponent } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    data: string;
};

export function StepEdit2({ position, track, data, ...props }: Props) {
    return getComponent('StepEdit2', position, [{ track }, { data }, props]);
}
