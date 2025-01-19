import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { BassTrack } from '../constants';
import { bottomLeftKnob, bottomRightKnob, topLeftKnob, topRightKnob } from '../constantsValue';
import { TextGridBass } from './TextGridBass';

export type Props = {
    name: string;
};

export function BassView({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="Volume VOLUME"
                position={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
                track={BassTrack}
            />
            <KnobValue
                value="303 CUTOFF"
                position={topRightKnob}
                encoder_id={2}
                COLOR="secondary"
                track={BassTrack}
            />
            <KnobValue
                value="303 ENV_MOD"
                position={bottomLeftKnob}
                encoder_id={1}
                track={BassTrack}
            />
            <KnobValue
                value="303 RESONANCE"
                position={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
                track={BassTrack}
            />

            <TextGridBass selected={0} viewName={name} />
            <Common selected={0} track={BassTrack} />
        </View>
    );
}
