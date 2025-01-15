import * as React from '@/libs/react';

import { SeqProgressBar } from '@/libs/components/SeqProgressBar';
import { SeqSynthBar } from '@/libs/components/SeqSynthBar';
import { rgb } from '@/libs/ui';
import { ScreenWidth } from '../constants';
import { TextGridCommon } from './TextGridCommon';

export function Common({
    selected,
    hideSequencer,
}: {
    selected: number;
    hideSequencer?: boolean;
}) {
    return (
        <>
            {!hideSequencer && (
                <SeqSynthBar position={[0, 270, ScreenWidth, 10]} seq_plugin="Sequencer" />
            )}
            <SeqProgressBar
                position={[0, 285, ScreenWidth, 5]}
                seq_plugin="Sequencer 0"
                active_color={rgb(35, 161, 35)}
                selection_color={rgb(35, 161, 35)}
                volume_plugin="Volume VOLUME"
            />
            <TextGridCommon selected={selected} />
        </>
    );
}
