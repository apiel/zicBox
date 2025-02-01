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

export function SynthWaveform2View({ name }: Props) {
    return (
        <View name={name}>
            <GraphEncoder
                position={topValues}
                plugin="Synth"
                data_id="WAVEFORM2"
                RENDER_TITLE_ON_TOP={true}
                encoders={[`${encTopLeft} SHAPE_2`, `${encTopRight} MORPH_2`]}
                track={SynthTrack}
            />

            <KnobValue
                value="Synth FREQ_2"
                position={bottomLeftKnob}
                encoder_id={encBottomLeft}
                COLOR="quaternary"
                track={SynthTrack}
            />

            <TextGridSynth selected={'E/Osc2'} viewName={name} />
            <Common selected={'Synth'} track={SynthTrack} selectedBackground={ColorTrack4} />
        </View>
    );
}
