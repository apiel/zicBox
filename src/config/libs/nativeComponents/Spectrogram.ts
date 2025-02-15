import { getComponent, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    data: string;
    text?: string;
};

export function Spectrogram({ bounds, track, data, text, ...props }: Props) {
    initializePlugin('Spectrogram', 'libzic_SpectrogramComponent.so');
    return getComponent('Spectrogram', bounds, [{ track }, { data, text }, props]);
}
