import { getComponent } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    data: string;
};

export function StepEditMono({ position, track, data, ...props }: Props) {
    return getComponent('StepEditMono', position, [{ track }, { data }, props]);
}
