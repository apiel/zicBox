import { getJsonComponent } from '../ui';

export const Pixels = getJsonComponent<{
    color?: string;
    count?: number;
}>('Pixels');