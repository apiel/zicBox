
import { getJsonComponent } from '../ui';

export const Pattern = getJsonComponent<{
    audioPlugin: string;
    bgColor?: string;
    color?: string;
    stepsDataId?: string;
    stepCountDataId?: string;
}>('Pattern');
