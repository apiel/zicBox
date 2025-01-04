import { getComponent } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    value: string;
};

export function Value({ position, track, value, ...props }: Props) {
    return getComponent('Value', position, [
        { track },
        { value },
        props,
    ]);
}
