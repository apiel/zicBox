import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { MacroEnvelop } from '@/libs/nativeComponents/MacroEnvelop';
import { Drum1, TracksSelector } from '../components/Common';
import { bounds1, enc3, enc4, enc7, enc8, enc9, graphBounds } from '../constantsValue';
import { DrumLayout } from './Layout';
import { ViewSelector } from './ViewSelector';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function Main2View({ name, track, synthName, color }: Props) {
    return (
        <DrumLayout
            viewName={name}
            color={color}
            synthName={synthName}
            content={
                <>
                    <MacroEnvelop
                        bounds={graphBounds(bounds1, 15)}
                        audioPlugin={synthName}
                        envelopDataId="ENV_FREQ2"
                        track={track}
                    />
                    <KnobValue
                        {...enc9}
                        audioPlugin={synthName}
                        param="DURATION"
                        color="quaternary"
                        track={track}
                    />

                    <KnobValue
                        audioPlugin={synthName}
                        param="OSC2"
                        {...enc3}
                        color="secondary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="OSC2_FREQ"
                        {...enc4}
                        color="tertiary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="LAYER2_CUTOFF"
                        {...enc7}
                        color="primary"
                        type="STRING"
                        track={track}
                        label='Osc 2 Filter'
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="LAYER2_RESONANCE"
                        {...enc8}
                        color="primary"
                        track={track}
                        label='Osc 2 Reso.'
                    />

                    <ViewSelector color={color} synthName={synthName} viewName={name} />
                    <TracksSelector
                        selectedBackground={color}
                        selected={Drum1}
                        trackName={Drum1}
                        viewName={name}
                        pageCount={2}
                        currentPage={2}
                    />
                </>
            }
        />
    );
}
