import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import { BassTrack, ColorTrack2, encBottomLeft, encTopLeft, encTopRight } from '../constants';
import {
    bottomLeftKnob,
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
                encoderId={encTopLeft}
                color="tertiary"
                track={BassTrack}
            />
            <KnobValue
                audioPlugin="Bass" param="DECAY_LEVEL"
                bounds={topRightKnob}
                encoderId={encTopRight}
                color="primary"
                track={BassTrack}
            />
            <KnobValue
                audioPlugin="Bass" param="DECAY_TIME"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="quaternary"
                track={BassTrack}
            />
            {/* <KnobValue
                audioPlugin="Bass" param="REVERB"
                bounds={bottomRightKnob}
                encoderId={encBottomRight}
                color="secondary"
                track={BassTrack}
            /> */}

            <TextGridBass selected={'Env'} viewName={name} />
            <Common selected={'Bass'} track={BassTrack} selectedBackground={ColorTrack2} />
        </View>
    );
}
