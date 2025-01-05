import { getComponent } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    plugin: string;
    envelop_data_id: string;
};

export function DrumEnvelop({ position, track, plugin, envelop_data_id, ...props }: Props) {
    return getComponent('DrumEnvelop', position, [
        { track },
        { plugin },
        {envelop_data_id},
        props,
    ]);
}
