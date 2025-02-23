import * as React from '@/libs/react';

import { DrumEnvelop } from '@/libs/nativeComponents/DrumEnvelop';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import {
    ColorTrack1,
    Drum23Track,
    encBottomLeft,
    encBottomRight,
    encTopLeft,
    encTopRight,
} from '../constants';
import { bottomRightKnob, topValues } from '../constantsValue';
import { TextGridDrum23 } from './TextGridDrum23';

export type Props = {
    name: string;
};

export function AmpView({ name }: Props) {
    return (
        <View name={name}>
            <DrumEnvelop
                bounds={topValues}
                audioPlugin="Drum23"
                envelopDataId="ENV_AMP"
                renderValuesOnTop={false}
                encoderTime={encTopLeft}
                encoderModulation={encTopRight}
                encoderPhase={encBottomLeft}
                track={Drum23Track}
            />
            <KnobValue
                bounds={bottomRightKnob}
                audioPlugin="Drum23" param="DURATION"
                encoderId={encBottomRight}
                color="quaternary"
                track={Drum23Track}
            />
            <TextGridDrum23 selected={'Amp'} viewName={name} />
            <Common selected={'Kick'} track={Drum23Track} selectedBackground={ColorTrack1} />
        </View>
    );
}
