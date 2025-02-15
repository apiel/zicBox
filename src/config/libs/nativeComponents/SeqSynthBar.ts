import { getComponent, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    seq_plugin: string;
};

export function SeqSynthBar({ bounds, track, seq_plugin, ...props }: Props) {
    initializePlugin('SeqSynthBar', 'libzic_SeqSynthBarComponent.so');
    return getComponent('SeqSynthBar', bounds, [
        { track },
        { seq_plugin },
        props,
    ]);
}
