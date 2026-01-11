import { getJsonComponent } from '../ui';

export const Play = getJsonComponent<{
    bgColor?: string;
    color?: string;
    playColor?: string;
    recColor?: string;
}>('Play');
