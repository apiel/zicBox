import * as React from '@/libs/react';

import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import { ColorTrack4, SynthTrack } from '../constants';
import { TextGridSynth } from './TextGridSynth';

export type Props = {
    name: string;
};

export function SynthWaveform2View({ name }: Props) {
    return (
        <View name={name}>
            {/* <GraphEncoder
                bounds={topValues}
                audioPlugin="Synth"
                dataId="WAVEFORM2"
                renderValuesOnTop={true}
                // encoders={[`${encTopLeft} SHAPE_2`, `${encTopRight} MORPH_2`]}
                // encoders={[
                //     {
                //         encoderId: encTopLeft,
                //         value: 'SHAPE_2',
                //     },
                //     {
                //         encoderId: encTopRight,
                //         value: 'MORPH_2',
                //     },
                // ]}
                values={['SHAPE_2', 'MORPH_2']}
                track={SynthTrack}
            /> */}

            {/* <KnobValue
                audioPlugin="Synth"
                param="FREQ_2"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="quaternary"
                track={SynthTrack}
            /> */}

            <TextGridSynth selected={'Osc2'} viewName={name} />
            <Common selected={'Synth'} track={SynthTrack} selectedBackground={ColorTrack4} />
        </View>
    );
}
