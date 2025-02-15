import { getComponent, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    data: string;
};

export function StepEditDrum({ bounds, track, data, ...props }: Props) {
    initializePlugin('StepEditDrum', 'libzic_StepEditDrumComponent.so');
    return getComponent('StepEditDrum', bounds, [{ track }, { data }, props]);
}
