import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import { BassTrack, ColorTrack2, encBottomLeft, encBottomRight, encTopLeft, encTopRight } from '../constants';
import {
    bottomLeftKnob,
    bottomRightKnob,
    topLeftKnob,
    topRightKnob
} from '../constantsValue';
import { TextGridBass } from './TextGridBass';

export type Props = {
    name: string;
};

export function BassView({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                audioPlugin="TrackFx" param="VOLUME"
                bounds={topLeftKnob}
                encoderId={encTopLeft}
                color="tertiary"
                track={BassTrack}
            />
            <KnobValue
                audioPlugin="Bass" param="CUTOFF"
                bounds={topRightKnob}
                encoderId={encTopRight}
                color="primary"
                track={BassTrack}
            />
            <KnobValue
                audioPlugin="Bass" param="FREQ_RATIO"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="quaternary"
                track={BassTrack}
            />
            <KnobValue
                audioPlugin="Bass" param="RESONANCE"
                bounds={bottomRightKnob}
                encoderId={encBottomRight}
                color="secondary"
                track={BassTrack}
            />

            <TextGridBass selected={'Main'} viewName={name} />
            <Common selected={'Bass'} track={BassTrack} selectedBackground={ColorTrack2} />
        </View>
    );
}
