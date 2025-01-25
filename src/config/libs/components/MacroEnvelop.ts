import { getComponent, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    plugin: string;
};

export function MacroEnvelop({ position, track, plugin, ...props }: Props) {
    initializePlugin('MacroEnvelop', 'libzic_MacroEnvelopComponent.so');
    return getComponent('MacroEnvelop', position, [
        { track },
        { plugin },
        props,
    ]);
}
