import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import {
    ColorTrack5,
    encBottomLeft,
    encBottomRight,
    encTopLeft,
    encTopRight,
    PercTrack
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
                audioPlugin="Perc" param="FM_FREQ"
                bounds={topLeftKnob}
                encoderId={encTopLeft}
                color="tertiary"
                track={PercTrack}
            />
            <KnobValue
                audioPlugin="Perc" param="FM_AMP"
                bounds={topRightKnob}
                encoderId={encTopRight}
                color="primary"
                track={PercTrack}
            />
            <KnobValue
                audioPlugin="Perc" param="ENV_MOD"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="quaternary"
                track={PercTrack}
            />
            <KnobValue
                audioPlugin="Perc" param="ENV_SHAPE"
                bounds={bottomRightKnob}
                encoderId={encBottomRight}
                color="secondary"
                track={PercTrack}
            />

            <TextGridPerc selected={'Mod.'} viewName={name} />
            <Common selected={'Perc'} track={PercTrack} selectedBackground={ColorTrack5}  synthName="Perc" />
        </View>
    );
}
