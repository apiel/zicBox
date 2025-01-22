import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { FmTrack } from '../constants';
import { bottomLeftKnob, bottomRightKnob, topLeftKnob, topRightKnob } from '../constantsValue';
import { TextGridFm } from './TextGridFm';

export type Props = {
    name: string;
};

export function FmView({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="TrackFx VOLUME"
                position={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
                track={FmTrack}
            />
            <KnobValue
                value="FmDrum CARRIER_FREQ"
                position={topRightKnob}
                encoder_id={2}
                COLOR="secondary"
                track={FmTrack}
            />
            <KnobValue
                value="FmDrum MOD_FREQ"
                position={bottomLeftKnob}
                encoder_id={1}
                track={FmTrack}
            />
            <KnobValue
                value="FmDrum MOD_INDEX"
                position={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
                track={FmTrack}
            />

            <TextGridFm selected={0} viewName={name} />
            <Common selected={3} track={FmTrack} />
        </View>
    );
}
