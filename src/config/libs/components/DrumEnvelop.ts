import { getComponent, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    plugin: string;
    envelop_data_id: string;
};

export function DrumEnvelop({ position, track, plugin, envelop_data_id, ...props }: Props) {
    initializePlugin('DrumEnvelop', 'libzic_DrumEnvelopComponent.so');
    return getComponent('DrumEnvelop', position, [
        { track },
        { plugin },
        {envelop_data_id},
        props,
    ]);
}
