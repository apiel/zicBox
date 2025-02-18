import { getJsonComponent } from '../ui';

export const SeqProgressBar = getJsonComponent<{
    audioPlugin: string;
    volumePlugin?: { plugin: string; param: string };
    bgColor?: string;
    fgColor?: string;
    activeColor?: string;
    inactiveStepColor?: string;
    selectionColor?: string;
    showSteps?: boolean;
}>('SeqProgressBar', 'libzic_SeqProgressBarComponent.so');
