import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import { BassTrack, ColorTrack2 } from '../constants';
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
                encoderId={0}
                color="tertiary"
                track={BassTrack}
            />
            <KnobValue
                audioPlugin="Bass" param="CUTOFF"
                bounds={topRightKnob}
                encoderId={1}
                color="primary"
                track={BassTrack}
            />
            <KnobValue
                audioPlugin="Bass" param="FREQ_RATIO"
                bounds={bottomLeftKnob}
                encoderId={2}
                color="quaternary"
                track={BassTrack}
            />
            <KnobValue
                audioPlugin="Bass" param="RESONANCE"
                bounds={bottomRightKnob}
                encoderId={3}
                color="secondary"
                track={BassTrack}
            />

            <TextGridBass selected={0} viewName={name} />
            <Common selected={3} track={BassTrack} selectedBackground={ColorTrack2} />
        </View>
    );
}
