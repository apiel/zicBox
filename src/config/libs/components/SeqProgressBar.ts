import { getComponent, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    seq_plugin: string;
};

export function SeqProgressBar({ position, track, seq_plugin, ...props }: Props) {
    initializePlugin('SeqProgressBar', 'libzic_SeqProgressBarComponent.so');
    return getComponent('SeqProgressBar', position, [{ track }, { seq_plugin }, props]);
}
