import { getComponent } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    plugin: string;
    data_id: string;
    encoders: string[];
};

export function GraphEncoder({ position, track, plugin, data_id, encoders, ...props }: Props) {
    return getComponent('GraphEncoder', position, [
        { track },
        { plugin },
        { data_id },
        encoders.map((encoder) => ({ encoder })),
        props,
    ]);
}
