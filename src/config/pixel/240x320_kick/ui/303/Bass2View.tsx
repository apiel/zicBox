import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { BassTrack } from '../constants';
import { topLeftKnob } from '../constantsValue';
import { TextGridBass } from './TextGridBass';

export type Props = {
    name: string;
};

export function Bass2View({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="303 DECAY"
                position={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
                track={BassTrack}
            />
            {/* <KnobValue
                value="303 GLIDE"
                position={topRightKnob}
                encoder_id={2}
                COLOR="secondary"
                track={BassTrack}
            /> */}
            {/* <KnobValue
                value="Snare TRANSIENT_DURATION"
                position={bottomLeftKnob}
                encoder_id={1}
                track={SnareTrack}
            /> */}
            {/* <KnobValue
                value="Snare TRANSIENT_INTENSITY"
                position={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
                track={SnareTrack}
            /> */}

            <TextGridBass selected={0} viewName={name} />
            <Common selected={0} track={BassTrack} />
        </View>
    );
}
