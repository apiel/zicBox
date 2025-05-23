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

export function ClickView({ name, track, synthName, color }: Props) {
    return (
        <View name={name}>
            <KnobValue
                audioPlugin={synthName}
                param="CLICK"
                bounds={topLeftKnob}
                encoderId={encTopLeft}
                color="tertiary"
                track={track}
            />
            <KnobValue
                audioPlugin={synthName}
                param="CLICK_CUTOFF"
                bounds={topRightKnob}
                encoderId={encTopRight}
                color="primary"
                track={track}
            />
            <KnobValue
                audioPlugin={synthName}
                param="CLICK_DURATION"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="quaternary"
                track={track}
            />
            <KnobValue
                audioPlugin={synthName}
                param="OSC2"
                bounds={bottomRightKnob}
                encoderId={encBottomRight}
                color="secondary"
                track={track}
            />

            <ViewSelector selected={'Click'} color={color} synthName={synthName} viewName={name} />
            <Common
                selected={synthName}
                track={track}
                selectedBackground={color}
                synthName={synthName}
            />
        </View>
    );
}
