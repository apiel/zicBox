import * as React from '@/libs/react';

import { SeqProgressBar } from '@/libs/components/SeqProgressBar';
import { rgb } from '@/libs/ui';
import { ScreenWidth } from '../constants';
import { TextGridCommon } from './TextGridCommon';

export function Common({ selected, hideSequencer }: { selected: number; hideSequencer?: boolean }) {
    return (
        <>
            {!hideSequencer && (
                <SeqProgressBar
                    position={[0, 0, ScreenWidth, 5]}
                    seq_plugin="Sequencer"
                    active_color={rgb(35, 161, 35)}
                    selection_color={rgb(35, 161, 35)}
                    foreground_color={rgb(34, 110, 34)}
                    volume_plugin="Volume VOLUME"
                    show_steps
                />
            )}

            <TextGridCommon selected={selected} />
        </>
    );
}
