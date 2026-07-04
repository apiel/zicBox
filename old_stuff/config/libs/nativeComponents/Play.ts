import { getJsonComponent } from '../ui';

export const Play = getJsonComponent<{
    bgColor?: string;
    color?: string;
    playColor?: string;
    recColor?: string;
    mode?: "DEFAULT" | "TOGGLE_PLAY_PAUSE" | "TOGGLE_PLAY_STOP" | "TOGGLE_RECORD";
}>('Play');
