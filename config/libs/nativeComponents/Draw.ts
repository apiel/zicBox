import { getJsonComponent } from '../ui';

export const Draw = getJsonComponent<{
    lines?: [number, number][];
    color?: string;
    bgColor?: string;
    thickness?: number;
}>('Draw');
