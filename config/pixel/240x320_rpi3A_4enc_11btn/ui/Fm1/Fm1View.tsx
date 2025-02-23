import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import { ColorTrack3, encBottomLeft, encBottomRight, encTopLeft, encTopRight, Fm1Track } from '../constants';
import {
    bottomLeftKnob,
    bottomRightKnob,
    topLeftKnob,
    topRightKnob
} from '../constantsValue';
import { TextGridFm1 } from './TextGridFm1';

export type Props = {
    name: string;
};

export function Fm1View({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                audioPlugin="TrackFx" param="VOLUME"
                bounds={topLeftKnob}
                encoderId={encTopLeft}
                color="tertiary"
                track={Fm1Track}
            />
            <KnobValue
                audioPlugin="FmDrum" param="CARRIER_FREQ"
                bounds={topRightKnob}
                encoderId={encTopRight}
                color="primary"
                track={Fm1Track}
            />
            <KnobValue
                audioPlugin="FmDrum" param="MOD_FREQ"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="quaternary"
                track={Fm1Track}
            />
            <KnobValue
                audioPlugin="FmDrum" param="MOD_INDEX"
                bounds={bottomRightKnob}
                encoderId={encBottomRight}
                color="secondary"
                track={Fm1Track}
            />

            <TextGridFm1 selected={'Main'} viewName={name} />
            <Common selected={'Fm1'} track={Fm1Track} selectedBackground={ColorTrack3} />
        </View>
    );
}
