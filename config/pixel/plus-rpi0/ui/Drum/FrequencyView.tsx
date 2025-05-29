import * as React from '@/libs/react';

import { MacroEnvelop } from '@/libs/nativeComponents/MacroEnvelop';
import { View } from '@/libs/nativeComponents/View';
import { TracksSelector } from '../components/Common';
import { fullValues } from '../constantsValue';
import { ViewSelector } from './ViewSelector';

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
            <ViewSelector selected={'Freq'} color={color} synthName={synthName} viewName={name} />
            <TracksSelector selectedBackground={color} viewName={name}  />
        </View>
    );
}
