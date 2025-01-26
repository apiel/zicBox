import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { ColorTrack1, Drum23Track } from '../constants';
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

export function ClickView({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="Drum23 CLICK"
                position={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
                track={Drum23Track}
            />
            <KnobValue
                value="Drum23 CLICK_CUTOFF"
                position={topRightKnob}
                encoder_id={2}
                COLOR="primary"
                track={Drum23Track}
            />
            <KnobValue
                value="Drum23 CLICK_DURATION"
                position={bottomLeftKnob}
                encoder_id={1}
                COLOR="quaternary"
                track={Drum23Track}
            />
            <KnobValue
                value="Drum23 HIGH_FREQ_BOOST"
                position={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
                track={Drum23Track}
            />

            <TextGridDrum23 selected={4} viewName={name} />
            <Common selected={0} track={Drum23Track} selectedBackground={ColorTrack1} />
        </View>
    );
}
