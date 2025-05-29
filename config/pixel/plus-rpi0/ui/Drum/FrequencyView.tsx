import * as React from '@/libs/react';

import { MacroEnvelop } from '@/libs/nativeComponents/MacroEnvelop';
import { Drum1, TracksSelector } from '../components/Common';
import { fullValues } from '../constantsValue';
import { Layout } from './Layout';
import { ViewSelector } from './ViewSelector';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function FrequencyView({ name, track, synthName, color }: Props) {
    return (
        <Layout
            viewName={name}
            color={color}
            content={
                <>
                    <MacroEnvelop
                        bounds={fullValues}
                        audioPlugin={synthName}
                        envelopDataId="ENV_FREQ2"
                        track={track}
                    />
                    <ViewSelector
                        selected={'Freq'}
                        color={color}
                        synthName={synthName}
                        viewName={name}
                    />
                    <TracksSelector selectedBackground={color} viewName={name} trackName={Drum1} />
                </>
            }
        />
    );
}
