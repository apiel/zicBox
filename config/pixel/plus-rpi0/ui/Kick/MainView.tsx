import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import {
    encBottomRight,
    encTopLeft,
    encTopRight
} from '../constants';
import { bottomRightKnob, topLeftKnob, topRightKnob } from '../constantsValue';
import { ViewSelector } from './ViewSelector';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function MainView({ name, track, synthName, color }: Props) {
    return (
        <View name={name}>
            <KnobValue
                audioPlugin="TrackFx"
                param="VOLUME"
                bounds={topLeftKnob}
                encoderId={encTopLeft}
                color="tertiary"
                track={track}
            />
            <KnobValue
                audioPlugin="MMFilter"
                param="CUTOFF"
                bounds={topRightKnob}
                encoderId={encTopRight}
                color="secondary"
                type="STRING"
                track={track}
            />
            {/* <KnobValue
                audioPlugin={synthName}
                param="GAIN_CLIPPING"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                track={track}
            /> */}
            <KnobValue
                audioPlugin="MMFilter"
                param="RESONANCE"
                bounds={bottomRightKnob}
                encoderId={encBottomRight}
                color="secondary"
                track={track}
            />

            <ViewSelector selected={'Main'} color={color} synthName={synthName} viewName={name} />
            <Common selected={synthName} track={track} selectedBackground={color} synthName={synthName} />
        </View>
    );
}
