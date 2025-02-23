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
}>('Workspaces');
