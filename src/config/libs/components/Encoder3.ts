import { getComponent } from '../ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    value: string;
    encoder_id: number;
};

export function Encoder3({ position, track, value, encoder_id, ...props }: Props) {
    return getComponent('Encoder3', position, [
        { track },
        { encoder_id },
        { value },
        props,
    ]);
}
