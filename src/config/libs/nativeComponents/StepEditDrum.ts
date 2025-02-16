import { getOldComponentToBeDeprecated, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    data: string;
};

export function StepEditDrum({ bounds, track, data, ...props }: Props) {
    initializePlugin('StepEditDrum', 'libzic_StepEditDrumComponent.so');
    return getOldComponentToBeDeprecated('StepEditDrum', bounds, [{ track }, { data }, props]);
}
