import { getComponent } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    data: string;
};

export function StepEditDrum({ position, track, data, ...props }: Props) {
    return getComponent('StepEditDrum', position, [{ track }, { data }, props]);
}
