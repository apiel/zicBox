import { getJsonComponent } from '../ui';

export const Rect = getJsonComponent<{
    color?: string;
    filled?: boolean;
}>('Rect');