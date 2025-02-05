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

export function SnareView({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="TrackFx VOLUME"
                position={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
                track={SnareTrack}
            />
            <KnobValue
                value="Snare DURATION"
                position={topRightKnob}
                encoder_id={2}
                COLOR="secondary"
                track={SnareTrack}
            />
            <KnobValue
                value="Snare TONE_FREQ"
                position={bottomLeftKnob}
                encoder_id={1}
                track={SnareTrack}
            />
            <KnobValue
                value="Snare NOISE_MIX"
                position={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
                track={SnareTrack}
            />

            <TextGridDrums selected={0} viewName={name} target="Snare" />
            <Common selected={1} track={SnareTrack} />
        </View>
    );
}
