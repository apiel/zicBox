import { getComponent, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    data: string;
};

export function StepEditMono({ position, track, data, ...props }: Props) {
    initializePlugin('StepEditMono', 'libzic_StepEditMonoComponent.so');
    return getComponent('StepEditMono', position, [{ track }, { data }, props]);
}
