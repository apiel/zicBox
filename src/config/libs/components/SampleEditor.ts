import { getComponent, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
};

export function SampleEditor({ position, track, ...props }: Props) {
    initializePlugin('SampleEditor', 'libzic_SampleEditorComponent.so');
    return getComponent('SampleEditor', position, [{ track }, props]);
}
