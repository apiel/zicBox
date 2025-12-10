import { getJsonComponent } from '../ui';

export const Icon = getJsonComponent<{
    name: string;
    color?: string;
    bgColor?: string;
    centered?: boolean;
    right?: boolean;
}>('Icon');
