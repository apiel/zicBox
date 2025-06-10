import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { TracksSelector } from '../components/Common';
import { Layout } from '../components/Layout';
import { encTopLeft, encTopRight } from '../constants';
import { topLeftKnob, topRightKnob } from '../constantsValue';
import { ViewSelector } from './ViewSelector';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function FxView({ name, track, synthName, color }: Props) {
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
                        param="FX_TYPE"
                        bounds={topLeftKnob}
                        encoderId={encTopLeft}
                        color="tertiary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin="TrackFx"
                        param="FX_AMOUNT"
                        bounds={topRightKnob}
                        encoderId={encTopRight}
                        color="secondary"
                        track={track}
                        valueReplaceTitle
                    />
                    {/* <KnobValue
                audioPlugin="TrackFx"
                param="FX_AMOUNT"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="quaternary"
                track={track}
            /> */}
                    {/* <KnobValue
                audioPlugin="Synth" param="MOD_INDEX"
                bounds={bottomRightKnob}
                encoderId={encBottomRight}
                color="secondary"
                track={track}
            /> */}

                    <ViewSelector
                        selected={'Fx'}
                        viewName={name}
                        synthName={synthName}
                        color={color}
                    />
                    <TracksSelector
                        selectedBackground={color}
                        viewName={name}
                        trackName={synthName}
                    />
                </>
            }
        />
    );
}
