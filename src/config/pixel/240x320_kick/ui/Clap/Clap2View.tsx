import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { ClapTrack } from '../constants';
import { bottomLeftKnob, bottomRightKnob, topLeftKnob, topRightKnob } from '../constantsValue';
import { TextGridClap } from './TextGridClap';

export type Props = {
    name: string;
};

export function Clap2View({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="Clap ATTACK_TIME"
                position={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
                track={ClapTrack}
            />
            <KnobValue
                value="Clap DECAY_TIME"
                position={topRightKnob}
                encoder_id={2}
                COLOR="secondary"
                track={ClapTrack}
            />
            <KnobValue
                value="Clap NOISE_LEVEL"
                position={bottomLeftKnob}
                encoder_id={1}
                track={ClapTrack}
            />
            <KnobValue
                value="Clap DISTORTION"
                position={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
                track={ClapTrack}
            />

            <TextGridClap selected={1} viewName={name} />
            <Common selected={1} track={ClapTrack} />
        </View>
    );
}
