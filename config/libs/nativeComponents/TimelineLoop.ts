import { getJsonComponent } from '../ui';

export const TimelineLoop = getJsonComponent<{
    timelinePlugin: string;
    timelineDataId?: string;
    background?: string;
    gridColor?: string;
    loopColor?: string;
    viewStepStartContextId?: number;
}>('TimelineLoop');
