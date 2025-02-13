import * as React from '@/libs/react';

import { GraphEncoder } from '@/libs/components/GraphEncoder';
import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { ColorTrack4, encBottomLeft, encTopLeft, encTopRight, SynthTrack } from '../constants';
import { bottomLeftKnob, topValues } from '../constantsValue';
import { TextGridSynth } from './TextGridSynth';

export type Props = {
    name: string;
};

export function SynthWaveform1View({ name }: Props) {
    return (
        <View name={name}>
            <GraphEncoder
                position={topValues}
                plugin="Synth"
                data_id="WAVEFORM1"
                RENDER_TITLE_ON_TOP={true}
                encoders={[`${encTopLeft} SHAPE_1`, `${encTopRight} MORPH_1`]}
                track={SynthTrack}
            />

            <KnobValue
                value="Synth FREQ_1"
                position={bottomLeftKnob}
                encoder_id={encBottomLeft}
                COLOR="quaternary"
                track={SynthTrack}
            />

            <TextGridSynth selected={'Osc1'} viewName={name} />
            <Common selected={'Synth'} track={SynthTrack} selectedBackground={ColorTrack4} />
        </View>
    );
}
