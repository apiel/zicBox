import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { TracksSelector } from '../components/Common';
import { Layout } from '../components/Layout';
import { encBottomRight, encTopLeft, encTopRight } from '../constants';
import { bottomRightKnob, topLeftKnob, topRightKnob } from '../constantsValue';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function MainView({ name, track, synthName, color }: Props) {
    return (
        <Layout
            viewName={name}
            color={color}
            synthName={synthName}
            title={synthName}
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
                        audioPlugin="Filter"
                        param="CUTOFF"
                        bounds={topRightKnob}
                        encoderId={encTopRight}
                        type="STRING"
                        color="primary"
                        track={track}
                    />
                    {/* <KnobValue
                audioPlugin="Filter"
                param="CUTOFF"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="primary"
                track={track}
            /> */}
                    <KnobValue
                        audioPlugin="Filter"
                        param="RESONANCE"
                        bounds={bottomRightKnob}
                        encoderId={encBottomRight}
                        color="primary"
                        track={track}
                    />

                    {/* <ViewSelector
                        selected={'Main'}
                        viewName={name}
                        synthName={synthName}
                        color={color}
                    /> */}
                    <TracksSelector
                        selectedBackground={color}
                        viewName={name}
                        trackName={synthName}
                        selected={synthName}
                    />
                </>
            }
        />
    );
}
