// import { getOldComponentToBeDeprecated, initializePlugin } from '@/libs/ui';
// import { ComponentProps } from './component';

// export type Props = ComponentProps & {
//     plugin: string;
// };

// export function Workspaces({ bounds, track, plugin, ...props }: Props) {
//     initializePlugin('Workspaces', 'libzic_WorkspacesComponent.so');
//     return getOldComponentToBeDeprecated('Workspaces', bounds, [{ track }, { plugin }, props]);
// }


import { getJsonComponent } from '../ui';

export const Workspaces = getJsonComponent<{
    audioPlugin?: string;
    bgColor?: string;
    textColor?: string;
    selectionColor?: string;
    itemBackground?: string;
    currentWorkspaceDataId?: string;
    refreshStateDataId?: string;
    workspaceFolder?: string;
    badgeColor?: string;
    errorColor?: string;
}>('Workspaces', 'libzic_WorkspacesComponent.so');
