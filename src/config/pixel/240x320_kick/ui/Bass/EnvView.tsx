import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import { BassTrack, ColorTrack2 } from '../constants';
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
                audioPlugin="Bass" param="DURATION"
                bounds={topLeftKnob}
                encoderId={0}
                color="tertiary"
                track={BassTrack}
            />
            <KnobValue
                audioPlugin="Bass" param="DECAY_LEVEL"
                bounds={topRightKnob}
                encoderId={1}
                color="primary"
                track={BassTrack}
            />
            <KnobValue
                audioPlugin="Bass" param="DECAY_TIME"
                bounds={bottomLeftKnob}
                encoderId={2}
                color="quaternary"
                track={BassTrack}
            />
            <KnobValue
                audioPlugin="Bass" param="REVERB"
                bounds={bottomRightKnob}
                encoderId={3}
                color="secondary"
                track={BassTrack}
            />

            <TextGridBass selected={1} viewName={name} />
            <Common selected={3} track={BassTrack} selectedBackground={ColorTrack2} />
        </View>
    );
}
