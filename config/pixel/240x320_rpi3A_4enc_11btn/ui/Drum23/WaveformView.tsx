import * as React from '@/libs/react';

import { GraphEncoder } from '@/libs/nativeComponents/GraphEncoder';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { Value } from '@/libs/nativeComponents/Value';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import {
    ColorTrack1,
    Drum23Track,
    encBottomLeft,
    encBottomRight,
    encTopLeft,
    encTopRight,
    ScreenWidth,
} from '../constants';
import { bottomRightKnob, graphTopValues } from '../constantsValue';
import { TextGridDrum23 } from './TextGridDrum23';

export type Props = {
    name: string;
};

export function WaveformView({ name }: Props) {
    return (
        <View name={name}>
            <GraphEncoder
                bounds={graphTopValues}
                audioPlugin="Drum23"
                dataId="WAVEFORM"
                renderValuesOnTop={false}
                values={['WAVEFORM_TYPE', 'SHAPE', 'MACRO']}
                track={Drum23Track}
            />

            <Value
                bounds={[0, 55, ScreenWidth / 2 - 2, 20]}
                audioPlugin="Drum23"
                param="WAVEFORM_TYPE"
                track={Drum23Track}
                fontLabel="PoppinsLight_6"
                barHeight={0}
                encoderId={encTopLeft}
                alignLeft
                showLabelOverValue={0}
            />

            <Value
                bounds={[ScreenWidth / 2 + 2, 55, ScreenWidth / 2, 20]}
                audioPlugin="Drum23"
                param="SHAPE"
                track={Drum23Track}
                fontLabel="PoppinsLight_6"
                barHeight={0}
                encoderId={encTopRight}
                alignLeft
                showLabelOverValue={0}
            />

            <Value
                bounds={[0, 145, ScreenWidth / 2 - 2, 20]}
                audioPlugin="Drum23"
                param="MACRO"
                track={Drum23Track}
                fontLabel="PoppinsLight_6"
                barHeight={0}
                encoderId={encBottomLeft}
                alignLeft
                showLabelOverValue={0}
            />

            <KnobValue
                bounds={bottomRightKnob}
                audioPlugin="Drum23"
                param="PITCH"
                encoderId={encBottomRight}
                color="secondary"
                track={Drum23Track}
            />
            <TextGridDrum23 selected={'Wave'} viewName={name} />
            <Common selected={'Kick'} track={Drum23Track} selectedBackground={ColorTrack1} />
        </View>
    );
}
