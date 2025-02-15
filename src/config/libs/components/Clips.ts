import { getComponent, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    plugin: string;
};

export function Clips({ bounds, track, ...props }: Props) {
    initializePlugin('Clips', 'libzic_ClipsComponent.so');
    return getComponent('Clips', bounds, [{ track }, props]);
}
