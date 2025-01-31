import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { ColorTrack3, encTopLeft, encTopRight, Fm1Track } from '../constants';
import {
    topLeftKnob,
    topRightKnob
} from '../constantsValue';
import { TextGridFm1 } from './TextGridFm1';

export type Props = {
    name: string;
};

export function Fm1EnvView({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="FmDrum ATTACK_TIME"
                position={topLeftKnob}
                encoder_id={encTopLeft}
                COLOR="tertiary"
                track={Fm1Track}
            />
            <KnobValue
                value="FmDrum DECAY_TIME"
                position={topRightKnob}
                encoder_id={encTopRight}
                COLOR="primary"
                track={Fm1Track}
            />
            {/* <KnobValue
                value="Bass DECAY_TIME"
                position={bottomLeftKnob}
                encoder_id={encBottomLeft}
                COLOR="quaternary"
                track={BassTrack}
            /> */}
            {/* <KnobValue
                value="Bass REVERB"
                position={bottomRightKnob}
                encoder_id={encBottomRight}
                COLOR="secondary"
                track={BassTrack}
            /> */}

            <TextGridFm1 selected={'Env'} viewName={name} />
            <Common selected={'Fm1'} track={Fm1Track} selectedBackground={ColorTrack3} />
        </View>
    );
}
