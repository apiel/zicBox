import { getJsonComponent } from '../ui';

export const GhRepo = getJsonComponent<{
    bgColor?: string;
    color?: string;
    fontSize?: number;
    font?: string;
}>('GhRepo');
