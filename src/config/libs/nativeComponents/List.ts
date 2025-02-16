import { getJsonComponent } from '../ui';

export const List = getJsonComponent<{
    items: string[];
    audioPlugin?: string;
    bgColor?: string;
    textColor?: string;
    selectionColor?: string;
    itemBackground?: string;
}>('List', 'libzic_ListComponent.so');
