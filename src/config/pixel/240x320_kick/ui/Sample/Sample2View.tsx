import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { TextGridDrums } from '../components/TextGridDrums';
import { SampleTrack } from '../constants';
import { bottomLeftKnob, bottomRightKnob, topLeftKnob, topRightKnob } from '../constantsValue';

export type Props = {
    name: string;
};

export function Sample2View({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="Sample RESONATOR"
                position={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
                track={SampleTrack}
            />
            <KnobValue
                value="Sample BOOST"
                position={topRightKnob}
                encoder_id={2}
                COLOR="secondary"
                track={SampleTrack}
            />
            <KnobValue
                value="Sample TIMBRE"
                position={bottomLeftKnob}
                encoder_id={1}
                track={SampleTrack}
            />
            <KnobValue
                value="Sample REVERB"
                position={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
                track={SampleTrack}
            />

            <TextGridDrums selected={1} viewName={name} target="Sample" />
            <Common selected={1} track={SampleTrack} />
        </View>
    );
}
