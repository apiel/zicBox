import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { Drum1, TracksSelector } from '../components/Common';
import { encBottomRight, encTopLeft, encTopRight } from '../constants';
import { bottomRightKnob, topLeftKnob, topRightKnob } from '../constantsValue';
import { DrumLayout } from './Layout';
import { ViewSelector } from './ViewSelector';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function MainView({ name, track, synthName, color }: Props) {
    return (
        <DrumLayout
            viewName={name}
            color={color}
            synthName={synthName}
            content={
                <>
                    <KnobValue
                        audioPlugin="TrackFx"
                        param="VOLUME"
                        bounds={topLeftKnob}
                        encoderId={encTopLeft}
                        color="tertiary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin="MMFilter"
                        param="CUTOFF"
                        bounds={topRightKnob}
                        encoderId={encTopRight}
                        color="secondary"
                        type="STRING"
                        track={track}
                    />
                    {/* <KnobValue
                audioPlugin={synthName}
                param="GAIN_CLIPPING"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                track={track}
            /> */}
                    <KnobValue
                        audioPlugin="MMFilter"
                        param="RESONANCE"
                        bounds={bottomRightKnob}
                        encoderId={encBottomRight}
                        color="secondary"
                        track={track}
                    />
                    <ViewSelector color={color} synthName={synthName} viewName={name} />
                    <TracksSelector
                        selectedBackground={color}
                        selected={Drum1}
                        viewName={name}
                        trackName={Drum1}
                        pageCount={2}
                        currentPage={1}
                    />
                </>
            }
        />
    );
}
