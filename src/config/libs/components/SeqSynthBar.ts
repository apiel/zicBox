import { getComponent } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    seq_plugin: string;
};

export function SeqSynthBar({ position, track, seq_plugin, ...props }: Props) {
    return getComponent('SeqSynthBar', position, [
        { track },
        { seq_plugin },
        props,
    ]);
}
