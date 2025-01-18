import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import {
    bottomLeftKnob,
    bottomRightKnob,
    topLeftKnob,
    topRightKnob
} from '../constantsValue';
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
                encoder_id={0}
                COLOR="tertiary"
            />
            <KnobValue
                value="Distortion COMPRESS"
                position={topRightKnob}
                encoder_id={2}
                COLOR="primary"
            />
            <KnobValue value="Distortion DRIVE" position={bottomLeftKnob} encoder_id={1} COLOR="quaternary" />
            <KnobValue
                value="Distortion BASS"
                position={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
            />
            <TextGridDrum23 selected={0} viewName={name} />
            <Common selected={0} />
        </View>
    );
}
