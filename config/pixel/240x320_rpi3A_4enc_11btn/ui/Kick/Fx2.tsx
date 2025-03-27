import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import { encTopLeft } from '../constants';
import { topLeftKnob } from '../constantsValue';
import { ViewSelector } from './ViewSelector';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function Fx2View({ name, track, synthName, color }: Props) {
    return (
        <View name={name}>
            <KnobValue
                audioPlugin={synthName}
                param="HIGH_FREQ_BOOST"
                bounds={topLeftKnob}
                encoderId={encTopLeft}
                color="tertiary"
                track={track}
            />
            {/* <KnobValue
                audioPlugin="Distortion" param="COMPRESS"
                bounds={topRightKnob}
                encoderId={encTopRight}
                color="primary"
                track={track}
            /> */}
            {/* <KnobValue
                audioPlugin="Distortion" param="DRIVE"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="quaternary"
                track={track}
            /> */}
            {/* <KnobValue
                audioPlugin="Distortion" param="BASS"
                bounds={bottomRightKnob}
                encoderId={encBottomRight}
                color="secondary"
                track={track}
            /> */}
            <ViewSelector selected={'Fx'} color={color} synthName={synthName} viewName={name} />
            <Common
                selected={synthName}
                track={track}
                selectedBackground={color}
                synthName={synthName}
            />
        </View>
    );
}
