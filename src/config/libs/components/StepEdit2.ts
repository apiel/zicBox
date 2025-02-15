import { getComponent, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    data: string;
};

export function StepEdit2({ bounds, track, data, ...props }: Props) {
    initializePlugin('StepEdit2', 'libzic_StepEdit2Component.so');
    return getComponent('StepEdit2', bounds, [{ track }, { data }, props]);
}
