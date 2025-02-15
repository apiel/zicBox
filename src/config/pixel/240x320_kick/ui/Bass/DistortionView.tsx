import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import { BassTrack, ColorTrack2 } from '../constants';
import { topLeftKnob, topRightKnob } from '../constantsValue';
import { TextGridBass } from './TextGridBass';

export type Props = {
    name: string;
};

export function BassDistortionView({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                audioPlugin="Bass" param="BOOST"
                bounds={topLeftKnob}
                encoderId={0}
                color="tertiary"
                track={BassTrack}
            />
            <KnobValue
                audioPlugin="Bass" param="GAIN_CLIPPING"
                bounds={topRightKnob}
                encoderId={1}
                color="primary"
                track={BassTrack}
            />
            {/* <KnobValue
                audioPlugin="Distortion" param="DRIVE"
                bounds={bottomLeftKnob}
                encoderId={2}
                color="quaternary"
                track={BassTrack}
            />
            <KnobValue
                audioPlugin="Distortion" param="BASS"
                bounds={bottomRightKnob}
                encoderId={3}
                color="secondary"
                track={BassTrack}
            /> */}
            <TextGridBass selected={0} viewName={name} />
            <Common selected={0} track={BassTrack} selectedBackground={ColorTrack2} />
        </View>
    );
}
