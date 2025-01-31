import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { ColorTrack3, encBottomLeft, encTopLeft, encTopRight, Fm1Track } from '../constants';
import { bottomLeftKnob, topLeftKnob, topRightKnob } from '../constantsValue';
import { TextGridBass } from './TextGridBass';

export type Props = {
    name: string;
};

export function Fm1FxView({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="FmDrum NOISE_LEVEL"
                position={topLeftKnob}
                encoder_id={encTopLeft}
                COLOR="tertiary"
                track={Fm1Track}
            />
            <KnobValue
                value="FmDrum DISTORTION"
                position={topRightKnob}
                encoder_id={encTopRight}
                COLOR="primary"
                track={Fm1Track}
            />
            <KnobValue
                value="FmDrum REVERB"
                position={bottomLeftKnob}
                encoder_id={encBottomLeft}
                COLOR="quaternary"
                track={Fm1Track}
            />
            {/* <KnobValue
                value="Distortion BASS"
                position={bottomRightKnob}
                encoder_id={encBottomRight}
                COLOR="secondary"
                track={BassTrack}
            /> */}
            <TextGridBass selected={'Fx'} viewName={name} />
            <Common selected={'Fm1'} track={Fm1Track} selectedBackground={ColorTrack3} />
        </View>
    );
}
