import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
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
                bounds={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
                track={HiHatTrack}
            />
            <KnobValue
                value="HiHat BAND_Q"
                bounds={topRightKnob}
                encoder_id={2}
                COLOR="secondary"
                track={HiHatTrack}
            />
            <KnobValue
                value="HiHat TONE_BRIGHTNESS"
                bounds={bottomLeftKnob}
                encoder_id={1}
                track={HiHatTrack}
            />
            {/* <KnobValue
                value="HiHat TONE_BRIGHTNESS"
                bounds={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
                track={HiHatTrack}
            /> */}

            <TextGridDrums selected={3} viewName={name} target="HiHat" />
            <Common selected={1} track={HiHatTrack} />
        </View>
    );
}
