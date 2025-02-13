import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { TextGridDrums } from '../components/TextGridDrums';
import { HiHatTrack } from '../constants';
import { bottomLeftKnob, bottomRightKnob, topLeftKnob, topRightKnob } from '../constantsValue';

export type Props = {
    name: string;
};

export function HiHatView({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="TrackFx VOLUME"
                position={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
                track={HiHatTrack}
            />
            <KnobValue
                value="HiHat DURATION"
                position={topRightKnob}
                encoder_id={2}
                COLOR="secondary"
                track={HiHatTrack}
            />
            <KnobValue
                value="HiHat TRANSIENT_INTENSITY"
                position={bottomLeftKnob}
                encoder_id={1}
                track={HiHatTrack}
            />
            <KnobValue
                value="HiHat METALLIC_TONE_MIX"
                position={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
                track={HiHatTrack}
            />

            <TextGridDrums selected={3} viewName={name} target="HiHat" />
            <Common selected={1} track={HiHatTrack} />
        </View>
    );
}
