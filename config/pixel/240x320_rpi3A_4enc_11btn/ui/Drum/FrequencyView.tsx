import * as React from '@/libs/react';

import { MacroEnvelop } from '@/libs/nativeComponents/MacroEnvelop';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import { fullValues } from '../constantsValue';
import { TextGridDrum } from './TextGridDrum';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function FrequencyView({ name, track, synthName, color }: Props) {
    return (
        <View name={name}>
            <MacroEnvelop
                bounds={fullValues}
                audioPlugin={synthName}
                envelopDataId="ENV_FREQ2"
                track={track}
            />
            <TextGridDrum selected={'Freq'} color={color} synthName={synthName} />
            <Common selected={synthName} track={track}  selectedBackground={color} synthName={synthName} />
        </View>
    );
}
