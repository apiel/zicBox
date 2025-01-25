import { getComponent, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    data: string;
};

export function StepEditSample({ position, track, data, ...props }: Props) {
    initializePlugin('StepEditSample', 'libzic_StepEditSampleComponent.so');
    return getComponent('StepEditSample', position, [{ track }, { data }, props]);
}
