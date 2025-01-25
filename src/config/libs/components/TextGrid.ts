import { getComponent, initializePlugin } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    rows: string[];
};

export function TextGrid({ position, track, rows, ...props }: Props) {
    initializePlugin('TextGrid', 'libzic_TextGridComponent.so');
    return getComponent('TextGrid', position, [
        { track },
        rows.map((row) => ({ ROW: row })),
        props,
    ]);
}
