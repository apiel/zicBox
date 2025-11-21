import { getJsonComponent } from '../ui';

export const Timeline = getJsonComponent<{
    workspaceFolder: string;
    timelineFilename: string;
    clipFolder: string;
    scrollEncoderId?: number;
    background?: string;
    gridColor?: string;
    barColor?: string;
    clipColor?: string;
    loopColor?: string;
    textColor?: string;
    selectedColor?: string;
    sequencerPlugin?: string;
    enginePlugin?: string;
}>('Timeline');
