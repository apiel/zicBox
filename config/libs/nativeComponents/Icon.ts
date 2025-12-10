import { getJsonComponent } from '../ui';

export const Icon = getJsonComponent<{
    name: string;
    color?: string;
    bgColor?: string;
}>('Icon');
