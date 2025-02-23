import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import { ColorTrack3, encBottomLeft, encTopLeft, encTopRight, Fm1Track } from '../constants';
import { bottomLeftKnob, topLeftKnob, topRightKnob } from '../constantsValue';
import { TextGridFm1 } from './TextGridFm1';

export type Props = {
    name: string;
};

export function Fm1FxView({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                audioPlugin="FmDrum" param="NOISE_LEVEL"
                bounds={topLeftKnob}
                encoderId={encTopLeft}
                color="tertiary"
                track={Fm1Track}
            />
            <KnobValue
                audioPlugin="FmDrum" param="DISTORTION"
                bounds={topRightKnob}
                encoderId={encTopRight}
                color="primary"
                track={Fm1Track}
            />
            <KnobValue
                audioPlugin="FmDrum" param="REVERB"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="quaternary"
                track={Fm1Track}
            />
            {/* <KnobValue
                audioPlugin="Distortion" param="BASS"
                bounds={bottomRightKnob}
                encoderId={encBottomRight}
                color="secondary"
                track={BassTrack}
            /> */}
            <TextGridFm1 selected={'Fx'} viewName={name} />
            <Common selected={'Fm1'} track={Fm1Track} selectedBackground={ColorTrack3} />
        </View>
    );
}
