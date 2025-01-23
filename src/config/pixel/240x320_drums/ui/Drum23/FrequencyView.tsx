import * as React from '@/libs/react';

import { MacroEnvelop } from '@/libs/components/MacroEnvelop';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { Drum23Track } from '../constants';
import { fullValues } from '../constantsValue';
import { TextGridDrum23 } from './TextGridDrum23';

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
                track={Drum23Track}
            />
            <TextGridDrum23 selected={4} viewName={name} />
            <Common selected={0} track={Drum23Track} />
        </View>
    );
}
