import { getComponent, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    value: string;
};

export function Value({ position, track, value, ...props }: Props) {
    initializePlugin('Value', 'libzic_ValueComponent.so');
    return getComponent('Value', position, [
        { track },
        { value },
        props,
    ]);
}
