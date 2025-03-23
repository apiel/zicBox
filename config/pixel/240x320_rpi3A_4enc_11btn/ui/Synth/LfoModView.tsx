import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import { encTopLeft } from '../constants';
import { topLeftKnob } from '../constantsValue';
import { ViewSelector } from './ViewSelector';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function LfoModView({ name, track, synthName, color }: Props) {
    return (
        <View name={name}>
            <KnobValue
                audioPlugin={synthName}
                param="LFO_FREQ_MOD"
                bounds={topLeftKnob}
                encoderId={encTopLeft}
                color="tertiary"
                track={track}
            />
            {/* <KnobValue
                audioPlugin={synthName}
                param="LFO_RATE"
                bounds={topRightKnob}
                encoderId={encTopRight}
                color="primary"
                track={track}
            /> */}
            {/* <KnobValue
                audioPlugin={synthName}
                param="LFO_FREQ"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="quaternary"
                track={track}
            /> */}
            {/* <KnobValue
                audioPlugin="TrackFx"
                param="FX_AMOUNT"
                bounds={bottomRightKnob}
                encoderId={encBottomRight}
                color="secondary"
                track={track}
            /> */}

            <ViewSelector selected={'Lfo.Mod'} viewName={name} synthName={synthName} color={color} />
            <Common track={track} selectedBackground={color} selected={synthName} synthName={synthName} />
        </View>
    );
}
