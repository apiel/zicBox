import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { ClapTrack } from '../constants';
import { topLeftKnob, topRightKnob } from '../constantsValue';
import { TextGridClap } from './TextGridClap';

export type Props = {
    name: string;
};

export function Clap2View({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="Clap SINE_FREQUENCY"
                position={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
                track={ClapTrack}
            />
            <KnobValue
                value="Clap SINE_BLEND"
                position={topRightKnob}
                encoder_id={2}
                COLOR="secondary"
                track={ClapTrack}
            />
            {/* <KnobValue
                value="Clap TONE_BRIGHTNESS"
                position={bottomLeftKnob}
                encoder_id={1}
                track={ClapTrack}
            /> */}
            {/* <KnobValue
                value="Clap TONE_BRIGHTNESS"
                position={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
                track={ClapTrack}
            /> */}

            <TextGridClap selected={1} viewName={name} />
            <Common selected={1} track={ClapTrack} />
        </View>
    );
}
