import { getJsonComponent } from '../ui';

export const GitHub = getJsonComponent<{
    bgColor?: string;
    textColor?: string;
    foregroundColor?: string;
    fontSize?: number;
    font?: string;
    encoderId?: number;
    contextId?: number;
}>('GitHub');
