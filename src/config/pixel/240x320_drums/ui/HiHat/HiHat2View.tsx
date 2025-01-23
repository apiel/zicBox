import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { TextGridDrums } from '../components/TextGridDrums';
import { HiHatTrack } from '../constants';
import { bottomLeftKnob, topLeftKnob, topRightKnob } from '../constantsValue';

export type Props = {
    name: string;
};

export function HiHat2View({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="HiHat BAND_FREQ"
                position={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
                track={HiHatTrack}
            />
            <KnobValue
                value="HiHat BAND_Q"
                position={topRightKnob}
                encoder_id={2}
                COLOR="secondary"
                track={HiHatTrack}
            />
            <KnobValue
                value="HiHat TONE_BRIGHTNESS"
                position={bottomLeftKnob}
                encoder_id={1}
                track={HiHatTrack}
            />
            {/* <KnobValue
                value="HiHat TONE_BRIGHTNESS"
                position={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
                track={HiHatTrack}
            /> */}

            <TextGridDrums selected={3} viewName={name} target="HiHat" />
            <Common selected={1} track={HiHatTrack} />
        </View>
    );
}
