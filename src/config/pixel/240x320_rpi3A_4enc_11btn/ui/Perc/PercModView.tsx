import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import {
    ColorTrack6,
    encBottomLeft,
    encBottomRight,
    encTopLeft,
    encTopRight,
    PercTrack,
} from '../constants';
import { bottomLeftKnob, bottomRightKnob, topLeftKnob, topRightKnob } from '../constantsValue';
import { TextGridPerc } from './TextGridPerc';

export type Props = {
    name: string;
};

export function PercModView({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="Perc FM_FREQ"
                bounds={topLeftKnob}
                encoder_id={encTopLeft}
                COLOR="tertiary"
                track={PercTrack}
            />
            <KnobValue
                value="Perc FM_AMP"
                bounds={topRightKnob}
                encoder_id={encTopRight}
                COLOR="primary"
                track={PercTrack}
            />
            <KnobValue
                value="Perc ENV_MOD"
                bounds={bottomLeftKnob}
                encoder_id={encBottomLeft}
                COLOR="quaternary"
                track={PercTrack}
            />
            <KnobValue
                value="Perc ENV_SHAPE"
                bounds={bottomRightKnob}
                encoder_id={encBottomRight}
                COLOR="secondary"
                track={PercTrack}
            />

            <TextGridPerc selected={'Mod.'} viewName={name} />
            <Common selected={'Perc'} track={PercTrack} selectedBackground={ColorTrack6} />
        </View>
    );
}
