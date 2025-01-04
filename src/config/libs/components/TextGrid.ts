import { getComponent } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    rows: string[];
};

export function TextGrid({ position, track, rows, ...props }: Props) {
    return getComponent('TextGrid', position, [
        { track },
        rows.map((row) => ({ ROW: row })),
        props,
    ]);
}
