import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import { encBottomRight, encTopLeft, encTopRight } from '../constants';
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
                audioPlugin="Filter"
                param="CUTOFF"
                bounds={topRightKnob}
                encoderId={encTopRight}
                type="STRING"
                color="primary"
                track={track}
            />
            {/* <KnobValue
                audioPlugin="Filter"
                param="CUTOFF"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="primary"
                track={track}
            /> */}
            <KnobValue
                audioPlugin="Filter"
                param="RESONANCE"
                bounds={bottomRightKnob}
                encoderId={encBottomRight}
                color="primary"
                track={track}
            />

            <ViewSelector selected={'Main'} viewName={name} synthName={synthName} color={color} />
            <Common
                track={track}
                selectedBackground={color}
                selected={synthName}
                synthName={synthName}
            />
        </View>
    );
}
