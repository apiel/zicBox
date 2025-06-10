import * as React from '@/libs/react';

import { DrumEnvelop } from '@/libs/nativeComponents/DrumEnvelop';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { Drum1, TracksSelector } from '../components/Common';
import { encBottomLeft, encBottomRight, encTopLeft, encTopRight } from '../constants';
import { bottomRightKnob, topValues } from '../constantsValue';
import { DrumLayout } from './Layout';
import { ViewSelector } from './ViewSelector';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function Layer2View({ name, track, synthName, color }: Props) {
    return (
        <DrumLayout
            viewName={name}
            color={color}
            synthName={synthName}
            content={
                <>
                    <DrumEnvelop
                        bounds={topValues}
                        audioPlugin={synthName}
                        envelopDataId="ENV_AMP_OSC2"
                        renderValuesOnTop={false}
                        encoderTime={encTopLeft}
                        encoderModulation={encTopRight}
                        encoderPhase={encBottomLeft}
                        track={track}
                    />
                    {/* <KnobValue
                audioPlugin={synthName}
                param="CLICK_CUTOFF"
                bounds={topLeftKnob}
                encoderId={encTopLeft}
                color="tertiary"
                track={track}
            /> */}
                    {/* <KnobValue
                audioPlugin={synthName}
                param="CLICK_CUTOFF"
                bounds={topRightKnob}
                encoderId={encTopRight}
                color="primary"
                track={track}
            /> */}
                    {/* <KnobValue
                audioPlugin={synthName}
                param="CLICK_DURATION"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="quaternary"
                track={track}
            /> */}
                    <KnobValue
                        audioPlugin={synthName}
                        param="OSC2"
                        bounds={bottomRightKnob}
                        encoderId={encBottomRight}
                        color="secondary"
                        track={track}
                    />

                    <ViewSelector
                        selected={'Layer2'}
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
