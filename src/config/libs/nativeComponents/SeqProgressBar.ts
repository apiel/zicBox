import { getOldComponentToBeDeprecated, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    seq_plugin: string;
};

export function SeqProgressBar({ bounds, track, seq_plugin, ...props }: Props) {
    initializePlugin('SeqProgressBar', 'libzic_SeqProgressBarComponent.so');
    return getOldComponentToBeDeprecated('SeqProgressBar', bounds, [{ track }, { seq_plugin }, props]);
}
