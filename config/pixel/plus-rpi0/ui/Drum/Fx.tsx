import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { Drum1, TracksSelector } from '../components/Common';
import { encBottomLeft, encBottomRight, encTopLeft, encTopRight } from '../constants';
import { bottomLeftKnob, bottomRightKnob, topLeftKnob, topRightKnob } from '../constantsValue';
import { DrumLayout } from './Layout';
import { ViewSelector } from './ViewSelector';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function FxView({ name, track, synthName, color }: Props) {
    return (
        <DrumLayout
            viewName={name}
            color={color}
            synthName={synthName}
            content={
                <>
                    <KnobValue
                        audioPlugin="Distortion"
                        param="WAVESHAPE"
                        bounds={topLeftKnob}
                        encoderId={encTopLeft}
                        color="tertiary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin="Distortion"
                        param="COMPRESS"
                        bounds={topRightKnob}
                        encoderId={encTopRight}
                        color="primary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin="Distortion"
                        param="DRIVE"
                        bounds={bottomLeftKnob}
                        encoderId={encBottomLeft}
                        color="quaternary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin="Distortion"
                        param="BASS"
                        bounds={bottomRightKnob}
                        encoderId={encBottomRight}
                        color="secondary"
                        track={track}
                    />
                    <ViewSelector
                        selected={'Fx'}
                        color={color}
                        synthName={synthName}
                        viewName={name}
                        pageCount={2}
                        currentPage={1}
                    />
                    <TracksSelector selectedBackground={color} viewName={name} trackName={Drum1} />
                </>
            }
        />
    );
}
