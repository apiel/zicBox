import * as React from '@/libs/react';

import { Spectrogram } from '@/libs/components/Spectrogram';
import { ScreenWidth } from '../constants';
import { ProgressBar } from './ProgressBar';
import { TextGridCommon } from './TextGridCommon';

export function Common({ selected }: { selected: number }) {
    return (
        <>
            <Spectrogram
                position={[0, 100, ScreenWidth, 70]}
                data="Spectrogram BUFFER"
                text="Pixel"
                raw_buffer
            />
            <ProgressBar />
            <TextGridCommon selected={selected} />
        </>
    );
}
