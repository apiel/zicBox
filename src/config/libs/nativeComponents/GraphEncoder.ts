import { getComponent, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    plugin: string;
    data_id: string;
    encoders: string[];
};

export function GraphEncoder({ bounds, track, plugin, data_id, encoders, ...props }: Props) {
    initializePlugin('GraphEncoder', 'libzic_GraphEncoderComponent.so');
    return getComponent('GraphEncoder', bounds, [
        { track },
        { plugin },
        { data_id },
        encoders.map((encoder) => ({ encoder })),
        props,
    ]);
}
