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

export function AmpView({ name, track, synthName, color }: Props) {
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
                        envelopDataId="ENV_AMP"
                        renderValuesOnTop={false}
                        encoderTime={encTopLeft}
                        encoderModulation={encTopRight}
                        encoderPhase={encBottomLeft}
                        track={track}
                    />
                    <KnobValue
                        bounds={bottomRightKnob}
                        audioPlugin={synthName}
                        param="DURATION"
                        encoderId={encBottomRight}
                        color="quaternary"
                        track={track}
                    />
                    <ViewSelector
                        selected={'Amp'}
                        color={color}
                        synthName={synthName}
                        viewName={name}
                    />
                    <TracksSelector selectedBackground={color} viewName={name} trackName={Drum1} />
                </>
            }
        />
    );
}
