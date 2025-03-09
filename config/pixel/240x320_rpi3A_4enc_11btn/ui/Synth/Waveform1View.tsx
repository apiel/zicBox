import * as React from '@/libs/react';

import { GraphEncoder } from '@/libs/nativeComponents/GraphEncoder';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import { ColorTrack4, encBottomLeft, SynthTrack } from '../constants';
import { bottomLeftKnob, topValues } from '../constantsValue';
import { TextGridSynth } from './TextGridSynth';

export type Props = {
    name: string;
};

export function SynthWaveform1View({ name }: Props) {
    return (
        <View name={name}>
            <GraphEncoder
                bounds={topValues}
                audioPlugin="Synth"
                dataId="WAVEFORM1"
                renderValuesOnTop={true}
                // encoders={[
                //     {
                //         encoderId: encTopLeft,
                //         value: 'SHAPE_1',
                //     },
                //     {
                //         encoderId: encTopRight,
                //         value: 'MORPH_1',
                //     },
                // ]}
                values={['SHAPE_1', 'MORPH_1']}
                track={SynthTrack}
            />

            <KnobValue
                audioPlugin="Synth" param="FREQ_1"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="quaternary"
                track={SynthTrack}
            />

            <TextGridSynth selected={'Osc1'} viewName={name} />
            <Common selected={'Synth'} track={SynthTrack} selectedBackground={ColorTrack4} />
        </View>
    );
}
