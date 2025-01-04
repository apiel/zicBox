import { getComponent } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    plugin: string;
    envelop_data_id: string;
    encoders: string[];
};

export function DrumEnvelop({ position, track, plugin, envelop_data_id, encoders, ...props }: Props) {
    return getComponent('DrumEnvelop', position, [
        { track },
        { plugin, envelop_data_id },
        encoders.map((encoder) => ({ encoder })),
        props,
    ]);
}
