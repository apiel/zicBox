import { getComponent } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    data: string;
};

export function StepEditSmall({ position, track, data, ...props }: Props) {
    return getComponent('StepEditSmall', position, [{ track }, { data }, props]);
}
