import { getComponent } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    data: string;
};

export function StepEditSample({ position, track, data, ...props }: Props) {
    return getComponent('StepEditSample', position, [{ track }, { data }, props]);
}
