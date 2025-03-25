import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import {
    encBottomLeft,
    encBottomRight,
    encTopLeft,
    encTopRight
} from '../constants';
import { bottomLeftKnob, bottomRightKnob, topLeftKnob, topRightKnob } from '../constantsValue';
import { TextGridDrum } from './TextGridDrum';

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
            <KnobValue
                audioPlugin={synthName}
                param="GAIN_CLIPPING"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                track={track}
            />
            <KnobValue
                audioPlugin="MMFilter"
                param="RESONANCE"
                bounds={bottomRightKnob}
                encoderId={encBottomRight}
                color="secondary"
                track={track}
            />

            <TextGridDrum selected={'Main'} color={color} synthName={synthName} />
            <Common selected={synthName} track={track} selectedBackground={color} synthName={synthName} />
        </View>
    );
}
