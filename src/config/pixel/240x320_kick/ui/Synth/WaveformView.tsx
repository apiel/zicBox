import * as React from '@/libs/react';

import { GraphEncoder } from '@/libs/components/GraphEncoder';
import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { W1_4, W3_4 } from '../constants';
import { encoderH, height } from '../constantsValue';
import { TextGridSynth } from './TextGridSynth';

export type Props = {
    name: string;
};

export function WaveformView({ name }: Props) {
    return (
        <View name={name}>
            <GraphEncoder
                position={[0, 0, W3_4 - 2, height]}
                plugin="Drum23"
                data_id="WAVEFORM"
                RENDER_TITLE_ON_TOP={false}
                encoders={['0 WAVEFORM_TYPE', '2 MACRO', '1 SHAPE']}
            />
            <KnobValue
                position={[W3_4, (height - encoderH) * 0.5, W1_4, encoderH]}
                value="Drum23 PITCH"
                encoder_id={3}
                color="secondary"
            />
            <TextGridSynth selected={1} viewName={name} />
            <Common selected={0} />
        </View>
    );
}
