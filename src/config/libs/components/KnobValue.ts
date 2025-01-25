import { getComponent, initializePlugin } from '../ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    value: string;
};

export function KnobValue({ position, track, value, ...props }: Props) {
    initializePlugin('KnobValue', 'libzic_KnobValueComponent.so');
    return getComponent('KnobValue', position, [
        { track },
        { value },
        props,
    ]);
}
