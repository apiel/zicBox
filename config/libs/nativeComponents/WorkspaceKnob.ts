import { getJsonComponent } from '../ui';

export const WorkspaceKnob = getJsonComponent<{
    bgColor?: string;
    color?: string;
    labelColor?: string;
    fontSize?: number;
    font?: string;
    labelFontSize?: number;
    labelFont?: string;
}>('WorkspaceKnob');
