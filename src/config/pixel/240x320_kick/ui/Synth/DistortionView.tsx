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
import { TextGridSynth } from './TextGridSynth';

export type Props = {
    name: string;
};

export function DistortionView({ name }: Props) {
    return (
        <View name={name}>
            {/* <Value value="Distortion WAVESHAPE" position={topLeft} encoder_id={0} {...tertiary} />
            <Value value="Distortion DRIVE" position={bottomLeft} encoder_id={1} {...primary} />
            <Value value="Distortion COMPRESS" position={topRight} encoder_id={2} {...secondary} />
            <Value value="Distortion BASS" position={bottomRight} encoder_id={3} {...quaternary} /> */}

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
            <TextGridSynth selected={0} viewName={name} />
            <Common selected={0} />
        </View>
    );
}
