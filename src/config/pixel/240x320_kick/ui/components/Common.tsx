import * as React from '@/libs/react';

import { SeqProgressBar } from '@/libs/components/SeqProgressBar';
import { Spectrogram } from '@/libs/components/Spectrogram';
import { ScreenWidth } from '../constants';
import { TextGridCommon } from './TextGridCommon';

export function Common({ selected }: { selected: number }) {
    return (
        <>
            <Spectrogram
                position={[0, 230, ScreenWidth, 50]}
                data="Spectrogram BUFFER"
                text="Pixel"
                // raw_buffer
            />
            <SeqProgressBar
                position={[0, 285, ScreenWidth, 5]}
                seq_plugin="Sequencer 0"
                active_color="#23a123"
                selection_color="#23a123"
                volume_plugin="Volume VOLUME"
            />
            <TextGridCommon selected={selected} />
        </>
    );
}
