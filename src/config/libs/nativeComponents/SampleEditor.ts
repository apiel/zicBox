import { getOldComponentToBeDeprecated, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
};

export function SampleEditor({ bounds, track, ...props }: Props) {
    initializePlugin('SampleEditor', 'libzic_SampleEditorComponent.so');
    return getOldComponentToBeDeprecated('SampleEditor', bounds, [{ track }, props]);
}
