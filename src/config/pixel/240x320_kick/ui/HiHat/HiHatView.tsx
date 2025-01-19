import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { HiHatTrack } from '../constants';
import { bottomLeftKnob, topLeftKnob, topRightKnob } from '../constantsValue';
import { TextGridHiHat } from './TextGridHiHat';

export type Props = {
    name: string;
};

export function HiHatView({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="Volume VOLUME"
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
            {/* <KnobValue
                value="HiHat NOISE_MIX"
                position={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
                track={HiHatTrack}
            /> */}

            <TextGridHiHat selected={3} viewName={name} />
            <Common selected={1} />
        </View>
    );
}
