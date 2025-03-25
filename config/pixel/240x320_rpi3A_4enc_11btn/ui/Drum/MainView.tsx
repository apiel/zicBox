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
                audioPlugin="Filter"
                param="FILTER_TYPE"
                bounds={topRightKnob}
                encoderId={encTopRight}
                color="secondary"
                track={track}
            />
            <KnobValue
                audioPlugin="Filter"
                param="FILTER_CUTOFF"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="primary"
                track={track}
            />
            <KnobValue
                audioPlugin="Filter"
                param="FILTER_RESONANCE"
                bounds={bottomRightKnob}
                encoderId={encBottomRight}
                color="primary"
                track={track}
            />

            <TextGridDrum selected={'Main'} color={color} synthName={synthName} />
            <Common selected={synthName} track={track} selectedBackground={color} synthName={synthName} />
        </View>
    );
}
