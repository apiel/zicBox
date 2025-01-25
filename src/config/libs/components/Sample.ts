import { getComponent, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    plugin: string;
};

export function Sample({ position, track, plugin, data_id, encoders, ...props }: Props) {
    initializePlugin('Sample', 'libzic_SampleComponent.so');
    return getComponent('Sample', position, [
        { track },
        { plugin },
        props,
    ]);
}
