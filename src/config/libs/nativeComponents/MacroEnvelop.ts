import { getOldComponentToBeDeprecated, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    plugin: string;
};

export function MacroEnvelop({ bounds, track, plugin, ...props }: Props) {
    initializePlugin('MacroEnvelop', 'libzic_MacroEnvelopComponent.so');
    return getOldComponentToBeDeprecated('MacroEnvelop', bounds, [
        { track },
        { plugin },
        props,
    ]);
}
