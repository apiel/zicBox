import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
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
                value="Drum23 CLICK"
                bounds={topLeftKnob}
                encoder_id={encTopLeft}
                COLOR="tertiary"
                track={Drum23Track}
            />
            <KnobValue
                value="Drum23 CLICK_CUTOFF"
                bounds={topRightKnob}
                encoder_id={encTopRight}
                COLOR="primary"
                track={Drum23Track}
            />
            <KnobValue
                value="Drum23 CLICK_DURATION"
                bounds={bottomLeftKnob}
                encoder_id={encBottomLeft}
                COLOR="quaternary"
                track={Drum23Track}
            />
            <KnobValue
                value="Drum23 HIGH_FREQ_BOOST"
                bounds={bottomRightKnob}
                encoder_id={encBottomRight}
                COLOR="secondary"
                track={Drum23Track}
            />

            <TextGridDrum23 selected={'Click'} viewName={name} />
            <Common selected={'Kick'} track={Drum23Track} selectedBackground={ColorTrack1} />
        </View>
    );
}
