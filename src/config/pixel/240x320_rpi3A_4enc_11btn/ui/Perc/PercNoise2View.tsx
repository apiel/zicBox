import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { ColorTrack6, encBottomLeft, encBottomRight, encTopLeft, encTopRight, PercTrack } from '../constants';
import {
    bottomLeftKnob,
    bottomRightKnob,
    topLeftKnob,
    topRightKnob
} from '../constantsValue';
import { TextGridPerc } from './TextGridPerc';

export type Props = {
    name: string;
};

export function PercNoise2View({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="Perc BAND_FREQ"
                position={topLeftKnob}
                encoder_id={encTopLeft}
                COLOR="tertiary"
                track={PercTrack}
            />
            <KnobValue
                value="Perc BAND_Q"
                position={topRightKnob}
                encoder_id={encTopRight}
                COLOR="primary"
                track={PercTrack}
            />
            <KnobValue
                value="Perc METALLIC_NOISE_MIX"
                position={bottomLeftKnob}
                encoder_id={encBottomLeft}
                COLOR="quaternary"
                track={PercTrack}
            />
            <KnobValue
                value="Perc NOISE_BRIGHTNESS"
                position={bottomRightKnob}
                encoder_id={encBottomRight}
                COLOR="secondary"
                track={PercTrack}
            />

            <TextGridPerc selected={'Noise2'} viewName={name} />
            <Common selected={'Perc'} track={PercTrack} selectedBackground={ColorTrack6} />
        </View>
    );
}
