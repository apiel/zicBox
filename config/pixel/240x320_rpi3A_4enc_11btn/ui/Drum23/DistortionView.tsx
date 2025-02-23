import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import { ColorTrack1, Drum23Track, encBottomLeft, encBottomRight, encTopLeft, encTopRight } from '../constants';
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
                encoderId={encTopLeft}
                color="tertiary"
                track={Drum23Track}
            />
            <KnobValue
                audioPlugin="Distortion" param="COMPRESS"
                bounds={topRightKnob}
                encoderId={encTopRight}
                color="primary"
                track={Drum23Track}
            />
            <KnobValue
                audioPlugin="Distortion" param="DRIVE"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="quaternary"
                track={Drum23Track}
            />
            <KnobValue
                audioPlugin="Distortion" param="BASS"
                bounds={bottomRightKnob}
                encoderId={encBottomRight}
                color="secondary"
                track={Drum23Track}
            />
            <TextGridDrum23 selected={'Fx'} viewName={name} />
            <Common selected={'Kick'} track={Drum23Track} selectedBackground={ColorTrack1} />
        </View>
    );
}
