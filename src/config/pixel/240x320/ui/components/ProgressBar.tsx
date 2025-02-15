import * as React from '@/libs/react';

import { ComponentProps } from '@/libs/nativeComponents/component';
import { SeqProgressBar } from '@/libs/nativeComponents/SeqProgressBar';
import { rgb } from '@/libs/ui';
import { ScreenWidth } from '../constants';

export type Props = Omit<ComponentProps, 'bounds'> & {
    y: number;
};

export function ProgressBar({ y, ...props }: Props) {
    return (
        <SeqProgressBar
            bounds={[0, y, ScreenWidth, 5]}
            seq_plugin="Sequencer 1"
            track={0}
            active_color={rgb(35, 161, 35)}
            foreground_color={rgb(35, 92, 35)}
            {...props}
        />
    );
}
