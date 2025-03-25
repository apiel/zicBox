import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import { encBottomLeft, encBottomRight, encTopLeft, encTopRight } from '../constants';
import { bottomLeftKnob, bottomRightKnob, topLeftKnob, topRightKnob } from '../constantsValue';
import { TextGridDrum } from './TextGridDrum';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function FxView({ name, track, synthName, color }: Props) {
    return (
        <View name={name}>
            <KnobValue
                audioPlugin={synthName}
                param="GAIN_CLIPPING"
                bounds={topLeftKnob}
                encoderId={encTopLeft}
                color="tertiary"
                track={track}
            />
            <KnobValue
                audioPlugin={synthName}
                param="HIGH_FREQ_BOOST"
                bounds={topRightKnob}
                encoderId={encTopRight}
                color="primary"
                track={track}
            />
            <KnobValue
                audioPlugin="TrackFx"
                param="FX_TYPE"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                track={track}
            />
            <KnobValue
                audioPlugin="TrackFx"
                param="FX_AMOUNT"
                bounds={bottomRightKnob}
                encoderId={encBottomRight}
                color="secondary"
                track={track}
            />
            <TextGridDrum selected={'Fx'} color={color} synthName={synthName} />
            <Common
                selected={synthName}
                track={track}
                selectedBackground={color}
                synthName={synthName}
            />
        </View>
    );
}
