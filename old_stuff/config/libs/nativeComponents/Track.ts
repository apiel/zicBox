import { getJsonComponent } from '../ui';

export const Track = getJsonComponent<{
    bgColor?: string;
    color?: string;
    iconSize?: number;
    sequencerPlugin?: string;
}>('Track');
