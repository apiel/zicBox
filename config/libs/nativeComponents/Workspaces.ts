import { getJsonComponent } from '../ui';

export const Workspaces = getJsonComponent<{
    audioPlugin?: string;
    bgColor?: string;
    textColor?: string;
    selectionColor?: string;
    itemBackground?: string;
    currentWorkspaceDataId?: string;
    repositoryFolderDataId?: string;
    refreshStateDataId?: string;
    badgeColor?: string;
    errorColor?: string;
}>('Workspaces');
