import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import { ColorTrack1, Drum23Track, encBottomLeft, encBottomRight, encTopLeft, encTopRight } from '../constants';
import {
    bottomLeftKnob,
    bottomRightKnob,
    topLeftKnob,
    topRightKnob
} from '../constantsValue';
import { TextGridDrum23 } from './TextGridDrum23';

export type Props = {
    name: string;
};

export function ClickView({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                audioPlugin="Drum23" param="CLICK"
                bounds={topLeftKnob}
                encoderId={encTopLeft}
                color="tertiary"
                track={Drum23Track}
            />
            <KnobValue
                audioPlugin="Drum23" param="CLICK_CUTOFF"
                bounds={topRightKnob}
                encoderId={encTopRight}
                color="primary"
                track={Drum23Track}
            />
            <KnobValue
                audioPlugin="Drum23" param="CLICK_DURATION"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="quaternary"
                track={Drum23Track}
            />
            <KnobValue
                audioPlugin="Drum23" param="HIGH_FREQ_BOOST"
                bounds={bottomRightKnob}
                encoderId={encBottomRight}
                color="secondary"
                track={Drum23Track}
            />

            <TextGridDrum23 selected={'Click'} viewName={name} />
            <Common selected={'Kick'} track={Drum23Track} selectedBackground={ColorTrack1} />
        </View>
    );
}
