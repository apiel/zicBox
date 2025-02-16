import { getOldComponentToBeDeprecated, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    data: string;
};

export function StepEditSample({ bounds, track, data, ...props }: Props) {
    initializePlugin('StepEditSample', 'libzic_StepEditSampleComponent.so');
    return getOldComponentToBeDeprecated('StepEditSample', bounds, [{ track }, { data }, props]);
}
