import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
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
                value="Distortion WAVESHAPE"
                position={topLeftKnob}
                encoder_id={encTopLeft}
                COLOR="tertiary"
                track={Drum23Track}
            />
            <KnobValue
                value="Distortion COMPRESS"
                position={topRightKnob}
                encoder_id={encTopRight}
                COLOR="primary"
                track={Drum23Track}
            />
            <KnobValue
                value="Distortion DRIVE"
                position={bottomLeftKnob}
                encoder_id={encBottomLeft}
                COLOR="quaternary"
                track={Drum23Track}
            />
            <KnobValue
                value="Distortion BASS"
                position={bottomRightKnob}
                encoder_id={encBottomRight}
                COLOR="secondary"
                track={Drum23Track}
            />
            <TextGridDrum23 selected={'Fx'} viewName={name} />
            <Common selected={'Kick'} track={Drum23Track} selectedBackground={ColorTrack1} />
        </View>
    );
}
