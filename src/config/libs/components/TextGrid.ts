import { getComponent, pluginComponent } from '@/libs/ui';
import { ComponentProps } from './component';

export type Props = ComponentProps & {
    rows: string[];
};

// pluginComponent

let initialized = false;

export function TextGrid({ position, track, rows, ...props }: Props) {
    if (!initialized) {
        initialized = true;
        pluginComponent('TextGrid', 'libzic_TextGridComponent.so');
    }
    return getComponent('TextGrid', position, [
        { track },
        rows.map((row) => ({ ROW: row })),
        props,
    ]);
}
