import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { BassTrack, ColorTrack2, encTopLeft, encTopRight } from '../constants';
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
                encoder_id={encTopLeft}
                COLOR="tertiary"
                track={BassTrack}
            />
            <KnobValue
                value="Bass GAIN_CLIPPING"
                position={topRightKnob}
                encoder_id={encTopRight}
                COLOR="primary"
                track={BassTrack}
            />
            {/* <KnobValue
                value="Distortion DRIVE"
                position={bottomLeftKnob}
                encoder_id={encBottomLeft}
                COLOR="quaternary"
                track={BassTrack}
            />
            <KnobValue
                value="Distortion BASS"
                position={bottomRightKnob}
                encoder_id={encBottomRight}
                COLOR="secondary"
                track={BassTrack}
            /> */}
            <TextGridBass selected={'Fx'} viewName={name} />
            <Common selected={'Bass'} track={BassTrack} selectedBackground={ColorTrack2} />
        </View>
    );
}
