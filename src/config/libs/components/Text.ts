import { getComponent, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
};

export function Text({ position, track, ...props }: Props) {
    initializePlugin('Text', 'libzic_TextComponent.so');
    return getComponent('Text', position, [{ track }, props]);
}
