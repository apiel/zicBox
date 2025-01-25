import { getComponent, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    plugin: string;
};

export function Workspaces({ position, track, plugin, ...props }: Props) {
    initializePlugin('Workspaces', 'libzic_WorkspacesComponent.so');
    return getComponent('Workspaces', position, [{ track }, { plugin }, props]);
}
