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

export function Drum23View({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="TrackFx VOLUME"
                position={topLeftKnob}
                encoder_id={encTopLeft}
                COLOR="tertiary"
                track={Drum23Track}
            />
            <KnobValue
                value="MMFilter CUTOFF"
                position={topRightKnob}
                encoder_id={encTopRight}
                COLOR="secondary"
                FONT_VALUE_SIZE={8}
                TYPE="STRING"
                track={Drum23Track}
            />
            <KnobValue
                value="Drum23 GAIN_CLIPPING"
                position={bottomLeftKnob}
                encoder_id={encBottomLeft}
                track={Drum23Track}
            />
            <KnobValue
                value="MMFilter RESONANCE"
                position={bottomRightKnob}
                encoder_id={encBottomRight}
                COLOR="secondary"
                track={Drum23Track}
            />

            <TextGridDrum23 selected={0} viewName={name} />
            <Common selected={0} track={Drum23Track} selectedBackground={ColorTrack1} />
        </View>
    );
}
