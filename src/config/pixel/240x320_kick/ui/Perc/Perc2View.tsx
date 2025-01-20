import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { TextGridDrums } from '../components/TextGridDrums';
import { PercTrack } from '../constants';
import { bottomLeftKnob, bottomRightKnob, topLeftKnob, topRightKnob } from '../constantsValue';

export type Props = {
    name: string;
};

export function Perc2View({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="Perc RESONATOR"
                position={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
                track={PercTrack}
            />
            <KnobValue
                value="Perc BOOST"
                position={topRightKnob}
                encoder_id={2}
                COLOR="secondary"
                track={PercTrack}
            />
            <KnobValue
                value="Perc TIMBRE"
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

            <TextGridDrums selected={1} viewName={name} target="Perc" />
            <Common selected={1} track={PercTrack} />
        </View>
    );
}
