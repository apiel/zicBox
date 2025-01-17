import { getComponent } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    plugin: string;
};

export function MacroEnvelop({ position, track, plugin, ...props }: Props) {
    return getComponent('MacroEnvelop', position, [
        { track },
        { plugin },
        props,
    ]);
}
