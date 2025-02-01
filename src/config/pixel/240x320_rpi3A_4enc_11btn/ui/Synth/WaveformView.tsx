import * as React from '@/libs/react';

import { GraphEncoder } from '@/libs/components/GraphEncoder';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { ColorTrack4, encBottomLeft, encBottomRight, encTopLeft, encTopRight, SynthTrack } from '../constants';
import { bottomValues, topValues } from '../constantsValue';
import { TextGridSynth } from './TextGridSynth';

export type Props = {
    name: string;
};

export function SynthWaveformView({ name }: Props) {
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

            <GraphEncoder
                position={bottomValues}
                plugin="Synth"
                data_id="WAVEFORM2"
                RENDER_TITLE_ON_TOP={true}
                encoders={[`${encBottomLeft} SHAPE_2`, `${encBottomRight} MORPH_2`]}
                track={SynthTrack}
            />

            <TextGridSynth selected={'Wave'} viewName={name} />
            <Common selected={'Synth'} track={SynthTrack} selectedBackground={ColorTrack4} />
        </View>
    );
}
