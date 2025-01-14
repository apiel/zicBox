import * as React from '@/libs/react';

import { DrumEnvelop } from '@/libs/components/DrumEnvelop';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { W3_4 } from '../constants';
import { height } from '../constantsValue';
import { TextGridSynth } from './TextGridSynth';

export type Props = {
    name: string;
};

export function FrequencyView({ name }: Props) {
    return (
        <View name={name}>
            <DrumEnvelop
                position={[0, 0, W3_4 - 2, height]}
                plugin="Drum23"
                envelop_data_id="4"
                RENDER_TITLE_ON_TOP={false}
                encoder_time={0}
                encoder_phase={1}
                encoder_modulation={2}
            />
            <TextGridSynth selected={4} viewName={name} />
            <Common selected={0} />
        </View>
    );
}
