import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
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
                value="TrackFx VOLUME"
                position={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
                track={BassTrack}
            />
            <KnobValue
                value="Bass CUTOFF"
                position={topRightKnob}
                encoder_id={2}
                COLOR="primary"
                track={BassTrack}
            />
            <KnobValue
                value="Bass FREQ_RATIO"
                position={bottomLeftKnob}
                encoder_id={1}
                COLOR="quaternary"
                track={BassTrack}
            />
            <KnobValue
                value="Bass RESONANCE"
                position={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
                track={BassTrack}
            />

            <TextGridBass selected={0} viewName={name} />
            <Common selected={3} track={BassTrack} selectedBackground={ColorTrack2} />
        </View>
    );
}
