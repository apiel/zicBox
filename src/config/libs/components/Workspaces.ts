import { getComponent } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    plugin: string;
};

export function Workspaces({ position, track, plugin, ...props }: Props) {
    return getComponent('Workspaces', position, [{ track }, { plugin }, props]);
}
