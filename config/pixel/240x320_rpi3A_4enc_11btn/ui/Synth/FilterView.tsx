import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import { encBottomLeft, encTopLeft, encTopRight } from '../constants';
import {
    bottomLeftKnob,
    topLeftKnob,
    topRightKnob
} from '../constantsValue';
import { TextGridSynth } from './TextGridSynth';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function SynthFilterView({ name, track, synthName, color }: Props) {
    return (
        <View name={name}>
            <KnobValue
                audioPlugin={synthName} param="FILTER_TYPE"
                bounds={topLeftKnob}
                encoderId={encTopLeft}
                color="tertiary"
                track={track}
            />
            <KnobValue
                audioPlugin={synthName} param="FILTER_CUTOFF"
                bounds={topRightKnob}
                encoderId={encTopRight}
                color="primary"
                track={track}
            />
            <KnobValue
                audioPlugin={synthName} param="FILTER_RESONANCE"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="quaternary"
                track={track}
            />
            {/* <KnobValue
                audioPlugin={synthName} param="MOD_INDEX"
                bounds={bottomRightKnob}
                encoderId={encBottomRight}
                color="secondary"
                track={track}
            /> */}

            <TextGridSynth selected={'Filter'} viewName={name} synthName={synthName} color={color} />
            <Common track={track} selectedBackground={color} selected={synthName} />
        </View>
    );
}
