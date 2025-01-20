import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { FmTrack } from '../constants';
import { bottomLeftKnob, bottomRightKnob, topLeftKnob, topRightKnob } from '../constantsValue';
import { TextGridFm } from './TextGridFm';

export type Props = {
    name: string;
};

export function Fm2View({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="FmDrum ATTACK_TIME"
                position={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
                track={FmTrack}
            />
            <KnobValue
                value="FmDrum DECAY_TIME"
                position={topRightKnob}
                encoder_id={2}
                COLOR="secondary"
                track={FmTrack}
            />
            {/* <KnobValue
                value="FmDrum NOISE_LEVEL"
                position={bottomLeftKnob}
                encoder_id={1}
                track={FmTrack}
            /> */}
            <KnobValue
                value="FmDrum DISTORTION"
                position={bottomLeftKnob}
                encoder_id={1}
                track={FmTrack}
            />
            <KnobValue
                value="FmDrum REVERB"
                position={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
                track={FmTrack}
            />

            <TextGridFm selected={1} viewName={name} />
            <Common selected={1} track={FmTrack} />
        </View>
    );
}
