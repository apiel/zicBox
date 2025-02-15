import { getComponent, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    plugin: string;
};

export function Workspaces({ bounds, track, plugin, ...props }: Props) {
    initializePlugin('Workspaces', 'libzic_WorkspacesComponent.so');
    return getComponent('Workspaces', bounds, [{ track }, { plugin }, props]);
}
