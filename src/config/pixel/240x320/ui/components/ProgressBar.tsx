import * as React from '@/libs/react';

import { ComponentProps } from '@/libs/components/component';
import { SeqProgressBar } from '@/libs/components/SeqProgressBar';
import { rgb } from '@/libs/ui';
import { ScreenWidth } from '../constants';

export type Props = Omit<ComponentProps, 'position'> & {
    y: number;
};

export function ProgressBar({ y, ...props }: Props) {
    return (
        <SeqProgressBar
            position={[0, y, ScreenWidth, 5]}
            seq_plugin="Sequencer 1"
            track={0}
            active_color={rgb(35, 161, 35)}
            foreground_color={rgb(35, 92, 35)}
            {...props}
        />
    );
}
