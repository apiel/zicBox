import * as React from '@/libs/react';

import { Log } from '@/libs/components/Log';
import { Rect } from '@/libs/components/Rect';
import { Spectrogram } from '@/libs/components/Spectrogram';
import { Value } from '@/libs/components/Value';
import { VisibilityContainer } from '@/libs/components/VisibilityContainer';
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
            <VisibilityContainer position={posTopContainer} group={0}>
                <Spectrogram
                    position={[0, 0, ScreenWidth, 70]}
                    track={0}
                    data="Spectrogram BUFFER"
                    text="Pixel"
                    raw_buffer
                />
            </VisibilityContainer>

            <VisibilityContainer position={posContainerValues} group={0}>
                <Log text="MasterValues" />
                <Rect position={[0, 0, ScreenWidth, height]} />
                <Value
                    value="MasterVolume VOLUME"
                    position={topLeft}
                    group={0}
                    track={0}
                    encoder_id={0}
                    {...tertiary}
                />
                <Value
                    value="MasterFilter CUTOFF"
                    position={topRight}
                    group={0}
                    track={0}
                    encoder_id={2}
                    {...quaternary}
                    USE_STRING_VALUE
                />
                <Value
                    value="MasterFilter RESONANCE"
                    position={bottomRight}
                    group={0}
                    track={0}
                    encoder_id={3}
                    {...quaternary}
                />
            </VisibilityContainer>
        </>
    );
}
