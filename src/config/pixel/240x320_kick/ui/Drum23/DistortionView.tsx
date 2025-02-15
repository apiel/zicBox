import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import { ColorTrack1, Drum23Track } from '../constants';
import { bottomLeftKnob, bottomRightKnob, topLeftKnob, topRightKnob } from '../constantsValue';
import { TextGridDrum23 } from './TextGridDrum23';

export type Props = {
    name: string;
};

export function DistortionView({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                audioPlugin="Distortion" param="WAVESHAPE"
                bounds={topLeftKnob}
                encoderId={0}
                color="tertiary"
                track={Drum23Track}
            />
            <KnobValue
                audioPlugin="Distortion" param="COMPRESS"
                bounds={topRightKnob}
                encoderId={1}
                color="primary"
                track={Drum23Track}
            />
            <KnobValue
                audioPlugin="Distortion" param="DRIVE"
                bounds={bottomLeftKnob}
                encoderId={2}
                color="quaternary"
                track={Drum23Track}
            />
            <KnobValue
                audioPlugin="Distortion" param="BASS"
                bounds={bottomRightKnob}
                encoderId={3}
                color="secondary"
                track={Drum23Track}
            />
            <TextGridDrum23 selected={0} viewName={name} />
            <Common selected={0} track={Drum23Track} selectedBackground={ColorTrack1} />
        </View>
    );
}
