import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import { encBottomLeft, encBottomRight, encTopLeft, encTopRight } from '../constants';
import { bottomLeftKnob, bottomRightKnob, topLeftKnob, topRightKnob } from '../constantsValue';
import { ViewSelector } from './ViewSelector';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function EnvLfoView({ name, track, synthName, color }: Props) {
    return (
        <View name={name}>
            <KnobValue
                audioPlugin={synthName}
                param="ATTACK"
                bounds={topLeftKnob}
                encoderId={encTopLeft}
                color="primary"
                track={track}
            />
            <KnobValue
                audioPlugin={synthName}
                param="RELEASE"
                bounds={topRightKnob}
                encoderId={encTopRight}
                color="primary"
                track={track}
            />
            <KnobValue
                audioPlugin={synthName}
                param="LFO_WAVEFORM"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="secondary"
                track={track}
            />
            <KnobValue
                audioPlugin={synthName}
                param="LFO_RATE"
                bounds={bottomRightKnob}
                encoderId={encBottomRight}
                color="secondary"
                track={track}
            />

            <ViewSelector selected={'Env/Lfo'} viewName={name} synthName={synthName} color={color} />
            <Common track={track} selectedBackground={color} selected={synthName} synthName={synthName} />
        </View>
    );
}
