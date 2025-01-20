import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { PercTrack } from '../constants';
import { bottomLeftKnob, bottomRightKnob, topLeftKnob, topRightKnob } from '../constantsValue';
import { TextGridPerc } from './TextGridPerc';

export type Props = {
    name: string;
};

export function Perc2View({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="Perc ATTACK_TIME"
                position={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
                track={PercTrack}
            />
            <KnobValue
                value="Perc DECAY_TIME"
                position={topRightKnob}
                encoder_id={2}
                COLOR="secondary"
                track={PercTrack}
            />
            {/* <KnobValue
                value="Perc NOISE_LEVEL"
                position={bottomLeftKnob}
                encoder_id={1}
                track={PercTrack}
            /> */}
            <KnobValue
                value="Perc DISTORTION"
                position={bottomLeftKnob}
                encoder_id={1}
                track={PercTrack}
            />
            <KnobValue
                value="Perc REVERB"
                position={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
                track={PercTrack}
            />

            <TextGridPerc selected={1} viewName={name} />
            <Common selected={1} track={PercTrack} />
        </View>
    );
}
