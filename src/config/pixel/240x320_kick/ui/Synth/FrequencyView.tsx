import * as React from '@/libs/react';

import { MacroEnvelop } from '@/libs/components/MacroEnvelop';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { fullValues } from '../constantsValue';
import { TextGridSynth } from './TextGridSynth';

export type Props = {
    name: string;
};

export function FrequencyView({ name }: Props) {
    return (
        <View name={name}>
            <MacroEnvelop
                position={fullValues}
                plugin="Drum23"
                envelop_data_id="ENV_FREQ2"
            />
            <TextGridSynth selected={4} viewName={name} />
            <Common selected={0} />
        </View>
    );
}
