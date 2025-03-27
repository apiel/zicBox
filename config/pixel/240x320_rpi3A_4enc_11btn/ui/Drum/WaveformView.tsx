import * as React from '@/libs/react';

import { GraphEncoder } from '@/libs/nativeComponents/GraphEncoder';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { Value } from '@/libs/nativeComponents/Value';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import {
    encBottomLeft,
    encBottomRight,
    encTopLeft,
    encTopRight,
    ScreenWidth
} from '../constants';
import { bottomRightKnob, graphTopValues } from '../constantsValue';
import { ViewSelector } from './ViewSelector';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function WaveformView({ name, track, synthName, color }: Props) {
    return (
        <View name={name}>
            <GraphEncoder
                bounds={graphTopValues}
                audioPlugin={synthName}
                dataId="WAVEFORM"
                renderValuesOnTop={false}
                values={['WAVEFORM_TYPE', 'SHAPE', 'MACRO']}
                track={track}
            />

            <Value
                bounds={[0, 55, ScreenWidth / 2 - 2, 22]}
                audioPlugin={synthName}
                param="WAVEFORM_TYPE"
                track={track}
                fontLabel="PoppinsLight_6"
                barHeight={1}
                encoderId={encTopLeft}
                alignLeft
                showLabelOverValue={0}
            />

            <Value
                bounds={[ScreenWidth / 2 + 2, 55, ScreenWidth / 2, 22]}
                audioPlugin={synthName}
                param="SHAPE"
                track={track}
                fontLabel="PoppinsLight_6"
                barHeight={1}
                encoderId={encTopRight}
                alignLeft
                showLabelOverValue={0}
            />

            <Value
                bounds={[0, 145, ScreenWidth / 2 - 2, 22]}
                audioPlugin={synthName}
                param="MACRO"
                track={track}
                fontLabel="PoppinsLight_6"
                barHeight={1}
                encoderId={encBottomLeft}
                alignLeft
                showLabelOverValue={0}
            />

            <KnobValue
                bounds={bottomRightKnob}
                audioPlugin={synthName}
                param="PITCH"
                encoderId={encBottomRight}
                color="secondary"
                track={track}
            />
            <ViewSelector selected={'Wave'} color={color} synthName={synthName} viewName={name} />
            <Common selected={synthName} track={track} selectedBackground={color} synthName={synthName} />
        </View>
    );
}
