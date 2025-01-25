import { getComponent, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    seq_plugin: string;
};

export function SeqSynthBar({ position, track, seq_plugin, ...props }: Props) {
    initializePlugin('SeqSynthBar', 'libzic_SeqSynthBarComponent.so');
    return getComponent('SeqSynthBar', position, [
        { track },
        { seq_plugin },
        props,
    ]);
}
