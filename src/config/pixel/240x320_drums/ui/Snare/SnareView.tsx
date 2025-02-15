import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
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
                bounds={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
                track={SnareTrack}
            />
            <KnobValue
                value="Snare DURATION"
                bounds={topRightKnob}
                encoder_id={2}
                COLOR="secondary"
                track={SnareTrack}
            />
            <KnobValue
                value="Snare TONE_FREQ"
                bounds={bottomLeftKnob}
                encoder_id={1}
                track={SnareTrack}
            />
            <KnobValue
                value="Snare NOISE_MIX"
                bounds={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
                track={SnareTrack}
            />

            <TextGridDrums selected={0} viewName={name} target="Snare" />
            <Common selected={1} track={SnareTrack} />
        </View>
    );
}
