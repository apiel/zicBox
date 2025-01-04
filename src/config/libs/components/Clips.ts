import { getComponent } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    plugin: string;
};

export function Clips({ position, track, ...props }: Props) {
    return getComponent('Clips', position, [{ track }, props]);
}
