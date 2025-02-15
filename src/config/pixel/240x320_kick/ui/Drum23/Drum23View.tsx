import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { ColorTrack1, Drum23Track } from '../constants';
import { bottomLeftKnob, bottomRightKnob, topLeftKnob, topRightKnob } from '../constantsValue';
import { TextGridDrum23 } from './TextGridDrum23';

export type Props = {
    name: string;
};

export function Drum23View({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="TrackFx VOLUME"
                bounds={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
                track={Drum23Track}
            />
            <KnobValue
                value="MMFilter CUTOFF"
                bounds={topRightKnob}
                encoder_id={1}
                COLOR="secondary"
                FONT_VALUE_SIZE={8}
                TYPE="STRING"
                track={Drum23Track}
            />
            <KnobValue
                value="Drum23 GAIN_CLIPPING"
                bounds={bottomLeftKnob}
                encoder_id={2}
                track={Drum23Track}
            />
            <KnobValue
                value="MMFilter RESONANCE"
                bounds={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
                track={Drum23Track}
            />

            <TextGridDrum23 selected={0} viewName={name} />
            <Common selected={0} track={Drum23Track} selectedBackground={ColorTrack1} />
        </View>
    );
}
