import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { BassTrack, ColorTrack2, encBottomRight, encTopLeft, encTopRight } from '../constants';
import { bottomRightKnob, topLeftKnob, topRightKnob } from '../constantsValue';
import { TextGridBass } from './TextGridBass';

export type Props = {
    name: string;
};

export function BassFxView({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="Bass BOOST"
                bounds={topLeftKnob}
                encoder_id={encTopLeft}
                COLOR="tertiary"
                track={BassTrack}
            />
            <KnobValue
                value="Bass GAIN_CLIPPING"
                bounds={topRightKnob}
                encoder_id={encTopRight}
                COLOR="primary"
                track={BassTrack}
            />
            {/* <KnobValue
                value="Distortion DRIVE"
                bounds={bottomLeftKnob}
                encoder_id={encBottomLeft}
                COLOR="quaternary"
                track={BassTrack}
            />
            <KnobValue
                value="Distortion BASS"
                bounds={bottomRightKnob}
                encoder_id={encBottomRight}
                COLOR="secondary"
                track={BassTrack}
            /> */}
            <KnobValue
                value="Bass REVERB"
                bounds={bottomRightKnob}
                encoder_id={encBottomRight}
                COLOR="secondary"
                track={BassTrack}
            />
            <TextGridBass selected={'Fx'} viewName={name} />
            <Common selected={'Bass'} track={BassTrack} selectedBackground={ColorTrack2} />
        </View>
    );
}
