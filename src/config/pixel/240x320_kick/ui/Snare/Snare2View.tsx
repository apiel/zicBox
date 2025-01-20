import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { TextGridDrums } from '../components/TextGridDrums';
import { SnareTrack } from '../constants';
import { bottomLeftKnob, bottomRightKnob, topLeftKnob, topRightKnob } from '../constantsValue';

export type Props = {
    name: string;
};

export function Snare2View({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="Snare PINK_NOISE"
                position={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
                track={SnareTrack}
            />
            <KnobValue
                value="Snare HARMONICS_COUNT"
                position={topRightKnob}
                encoder_id={2}
                COLOR="secondary"
                track={SnareTrack}
            />
            <KnobValue
                value="Snare TRANSIENT_DURATION"
                position={bottomLeftKnob}
                encoder_id={1}
                track={SnareTrack}
            />
            <KnobValue
                value="Snare TRANSIENT_INTENSITY"
                position={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
                track={SnareTrack}
            />

            <TextGridDrums selected={0} viewName={name} title="Snare" />
            <Common selected={1} track={SnareTrack} />
        </View>
    );
}
