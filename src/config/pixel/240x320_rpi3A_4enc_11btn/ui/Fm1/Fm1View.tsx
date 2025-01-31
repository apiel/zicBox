import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { ColorTrack3, encBottomLeft, encBottomRight, encTopLeft, encTopRight, Fm1Track } from '../constants';
import {
    bottomLeftKnob,
    bottomRightKnob,
    topLeftKnob,
    topRightKnob
} from '../constantsValue';
import { TextGridBass } from './TextGridBass';

export type Props = {
    name: string;
};

export function Fm1View({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="TrackFx VOLUME"
                position={topLeftKnob}
                encoder_id={encTopLeft}
                COLOR="tertiary"
                track={Fm1Track}
            />
            <KnobValue
                value="FmDrum CARRIER_FREQ"
                position={topRightKnob}
                encoder_id={encTopRight}
                COLOR="primary"
                track={Fm1Track}
            />
            <KnobValue
                value="FmDrum MOD_FREQ"
                position={bottomLeftKnob}
                encoder_id={encBottomLeft}
                COLOR="quaternary"
                track={Fm1Track}
            />
            <KnobValue
                value="FmDrum MOD_INDEX"
                position={bottomRightKnob}
                encoder_id={encBottomRight}
                COLOR="secondary"
                track={Fm1Track}
            />

            <TextGridBass selected={'Main'} viewName={name} />
            <Common selected={'Fm1'} track={Fm1Track} selectedBackground={ColorTrack3} />
        </View>
    );
}
