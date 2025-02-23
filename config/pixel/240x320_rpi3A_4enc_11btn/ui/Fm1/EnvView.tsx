import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
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
                audioPlugin="FmDrum" param="ATTACK_TIME"
                bounds={topLeftKnob}
                encoderId={encTopLeft}
                color="tertiary"
                track={Fm1Track}
            />
            <KnobValue
                audioPlugin="FmDrum" param="DECAY_TIME"
                bounds={topRightKnob}
                encoderId={encTopRight}
                color="primary"
                track={Fm1Track}
            />
            {/* <KnobValue
                audioPlugin="Bass" param="DECAY_TIME"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="quaternary"
                track={BassTrack}
            /> */}
            {/* <KnobValue
                audioPlugin="Bass" param="REVERB"
                bounds={bottomRightKnob}
                encoderId={encBottomRight}
                color="secondary"
                track={BassTrack}
            /> */}

            <TextGridFm1 selected={'Env'} viewName={name} />
            <Common selected={'Fm1'} track={Fm1Track} selectedBackground={ColorTrack3} />
        </View>
    );
}
