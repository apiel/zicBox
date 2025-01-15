import { getComponent } from '../ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    value: string;
};

export function KnobValue({ position, track, value, ...props }: Props) {
    return getComponent('KnobValue', position, [
        { track },
        { value },
        props,
    ]);
}
