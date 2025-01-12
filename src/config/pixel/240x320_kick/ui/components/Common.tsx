import * as React from '@/libs/react';

import { SeqProgressBar } from '@/libs/components/SeqProgressBar';
import { Spectrogram } from '@/libs/components/Spectrogram';
import { rgb, rgba } from '@/libs/ui';
import { ScreenWidth } from '../constants';
import { TextGridCommon } from './TextGridCommon';

export function Common({ selected }: { selected: number }) {
    return (
        <>
            <Spectrogram
                position={[0, 245, ScreenWidth, 35]}
                data="Spectrogram BUFFER"
                text="Pixel"
                // wave_color={rgb(35, 73, 117)}
                // wave_color={rgba(35, 73, 117, 0.6)}
                wave_color={rgba(35, 105, 117, 0.7)}
                raw_buffer
            />
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
