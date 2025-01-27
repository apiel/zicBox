import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { BassTrack, ColorTrack3 } from '../constants';
import {
    bottomLeftKnob,
    bottomRightKnob,
    topLeftKnob,
    topRightKnob
} from '../constantsValue';
import { TextGridBass } from './TextGridBass';

export type Props = {
    name: string;
};

export function BassEnvView({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="Bass DURATION"
                position={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
                track={BassTrack}
            />
            <KnobValue
                value="Bass DECAY_LEVEL"
                position={topRightKnob}
                encoder_id={2}
                COLOR="primary"
                track={BassTrack}
            />
            <KnobValue
                value="Bass DECAY_TIME"
                position={bottomLeftKnob}
                encoder_id={1}
                COLOR="quaternary"
                track={BassTrack}
            />
            <KnobValue
                value="Bass REVERB"
                position={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
                track={BassTrack}
            />

            <TextGridBass selected={1} viewName={name} />
            <Common selected={3} track={BassTrack} selectedBackground={ColorTrack3} />
        </View>
    );
}
