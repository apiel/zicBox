import * as React from '@/libs/react';

import { DrumEnvelop } from '@/libs/nativeComponents/DrumEnvelop';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import {
    encBottomLeft,
    encBottomRight,
    encTopLeft,
    encTopRight
} from '../constants';
import { bottomRightKnob, topValues } from '../constantsValue';
import { TextGridDrum23 } from './TextGridDrum23';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function AmpView({ name, track, synthName, color }: Props) {
    return (
        <View name={name}>
            <DrumEnvelop
                bounds={topValues}
                audioPlugin={synthName}
                envelopDataId="ENV_AMP"
                renderValuesOnTop={false}
                encoderTime={encTopLeft}
                encoderModulation={encTopRight}
                encoderPhase={encBottomLeft}
                track={track}
            />
            <KnobValue
                bounds={bottomRightKnob}
                audioPlugin={synthName} param="DURATION"
                encoderId={encBottomRight}
                color="quaternary"
                track={track}
            />
            <TextGridDrum23 selected={'Amp'} color={color} synthName={synthName} />
            <Common selected={synthName} track={track} selectedBackground={color} />
        </View>
    );
}
