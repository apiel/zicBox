import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { BassTrack, ColorTrack2, encBottomLeft, encTopLeft, encTopRight } from '../constants';
import {
    bottomLeftKnob,
    topLeftKnob,
    topRightKnob
} from '../constantsValue';
import { TextGridBass } from './TextGridBass';

export type Props = {
    name: string;
};

export function BassEnvView({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="Bass DURATION"
                bounds={topLeftKnob}
                encoder_id={encTopLeft}
                COLOR="tertiary"
                track={BassTrack}
            />
            <KnobValue
                value="Bass DECAY_LEVEL"
                bounds={topRightKnob}
                encoder_id={encTopRight}
                COLOR="primary"
                track={BassTrack}
            />
            <KnobValue
                value="Bass DECAY_TIME"
                bounds={bottomLeftKnob}
                encoder_id={encBottomLeft}
                COLOR="quaternary"
                track={BassTrack}
            />
            {/* <KnobValue
                value="Bass REVERB"
                bounds={bottomRightKnob}
                encoder_id={encBottomRight}
                COLOR="secondary"
                track={BassTrack}
            /> */}

            <TextGridBass selected={'Env'} viewName={name} />
            <Common selected={'Bass'} track={BassTrack} selectedBackground={ColorTrack2} />
        </View>
    );
}
