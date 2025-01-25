import { getComponent, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    data: string;
    text?: string;
};

export function Spectrogram({ position, track, data, text, ...props }: Props) {
    initializePlugin('Spectrogram', 'libzic_SpectrogramComponent.so');
    return getComponent('Spectrogram', position, [{ track }, { data, text }, props]);
}
