import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { Drum23Track } from '../constants';
import {
    topLeftKnob,
    topRightKnob
} from '../constantsValue';
import { TextGridDrum23 } from './TextGridDrum23';

export type Props = {
    name: string;
};

export function ResonatorView({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="Drum23 RESONATOR"
                position={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
                track={Drum23Track}
            />
            <KnobValue
                value="Drum23 TONE_DECAY"
                position={topRightKnob}
                encoder_id={2}
                COLOR="primary"
                track={Drum23Track}
            />
            {/* <KnobValue
                value="Drum23 CLICK_DURATION"
                position={bottomLeftKnob}
                encoder_id={1}
                COLOR="quaternary"
                track={Drum23Track}
            />
            <KnobValue
                value="Drum23 RESONATOR"
                position={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
                track={Drum23Track}
            /> */}

            <TextGridDrum23 selected={3} viewName={name} />
            <Common selected={0} track={Drum23Track} />
        </View>
    );
}
