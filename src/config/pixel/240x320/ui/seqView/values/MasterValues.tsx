import * as React from '@/libs/react';

import { Log } from '@/libs/nativeComponents/Log';
import { Rect } from '@/libs/nativeComponents/Rect';
import { Spectrogram } from '@/libs/nativeComponents/Spectrogram';
import { Value } from '@/libs/nativeComponents/Value';
import { VisibilityContainer } from '@/libs/nativeComponents/VisibilityContainer';
import { ScreenWidth } from '@/pixel/240x320/ui/constants';
import {
    bottomRight,
    height,
    posContainerValues,
    posTopContainer,
    quaternary,
    tertiary,
    topLeft,
    topRight,
} from './constants';

export function MasterValues() {
    return (
        <>
            <VisibilityContainer bounds={posTopContainer} group={0}>
                <Spectrogram
                    bounds={[0, 0, ScreenWidth, 70]}
                    track={0}
                    data="Spectrogram BUFFER"
                    text="Pixel"
                    raw_buffer
                />
            </VisibilityContainer>

            <VisibilityContainer bounds={posContainerValues} group={0}>
                <Log text="MasterValues" />
                <Rect bounds={[0, 0, ScreenWidth, height]} />
                <Value
                    value="MasterVolume VOLUME"
                    bounds={topLeft}
                    group={0}
                    track={0}
                    encoder_id={0}
                    {...tertiary}
                />
                <Value
                    value="MasterFilter CUTOFF"
                    bounds={topRight}
                    group={0}
                    track={0}
                    encoder_id={2}
                    {...quaternary}
                    USE_STRING_VALUE
                />
                <Value
                    value="MasterFilter RESONANCE"
                    bounds={bottomRight}
                    group={0}
                    track={0}
                    encoder_id={3}
                    {...quaternary}
                />
            </VisibilityContainer>
        </>
    );
}
