import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { BassTrack, ColorTrack2 } from '../constants';
import { topLeftKnob, topRightKnob } from '../constantsValue';
import { TextGridBass } from './TextGridBass';

export type Props = {
    name: string;
};

export function BassDistortionView({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="Bass BOOST"
                position={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
                track={BassTrack}
            />
            <KnobValue
                value="Bass GAIN_CLIPPING"
                position={topRightKnob}
                encoder_id={2}
                COLOR="primary"
                track={BassTrack}
            />
            {/* <KnobValue
                value="Distortion DRIVE"
                position={bottomLeftKnob}
                encoder_id={1}
                COLOR="quaternary"
                track={BassTrack}
            />
            <KnobValue
                value="Distortion BASS"
                position={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
                track={BassTrack}
            /> */}
            <TextGridBass selected={0} viewName={name} />
            <Common selected={0} track={BassTrack} selectedBackground={ColorTrack2} />
        </View>
    );
}
