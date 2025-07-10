import * as React from '@/libs/react';

import { GraphEncoder } from '@/libs/nativeComponents/GraphEncoder';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { MacroEnvelop } from '@/libs/nativeComponents/MacroEnvelop';
import { Value } from '@/libs/nativeComponents/Value';
import { rgb } from '@/libs/ui';
import { Drum1, TracksSelector } from '../components/Common';
import {
    bounds1,
    bounds10,
    bounds12,
    boundsMarginTop,
    enc1,
    enc10,
    enc11,
    enc12,
    enc2,
    enc3,
    enc4,
    enc5,
    enc6,
    enc7,
    enc8,
    enc9,
    encBottomValue,
    graphBounds,
} from '../constantsValue';
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
                        encoders={[enc1.encoderId, enc2.encoderId, enc5.encoderId, enc6.encoderId]}
                        title="Freq. Env."
                    />
                    <KnobValue
                        {...enc9}
                        audioPlugin={synthName}
                        param="DURATION"
                        color="quaternary"
                        track={track}
                    />
                    <GraphEncoder
                        bounds={boundsMarginTop(bounds10)}
                        audioPlugin={synthName}
                        dataId="ENV_AMP_FORM"
                        values={['AMP_MORPH']}
                        outlineColor="quaternary"
                        fillColor={rgb(194, 175, 107)}
                        track={track}
                    />
                    <Value
                        {...encBottomValue(enc10)}
                        audioPlugin={synthName}
                        param="AMP_MORPH"
                        track={track}
                        barHeight={1}
                        barColor="quaternary"
                        alignLeft
                        showLabelOverValue={1}
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
                        color="quaternary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="LAYER2_CUTOFF"
                        {...enc7}
                        color="tertiary"
                        type="STRING"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="LAYER2_RESONANCE"
                        {...enc8}
                        color="tertiary"
                        track={track}
                    />

                    <KnobValue
                        {...enc11}
                        audioPlugin={synthName}
                        param="LAYER2_DURATION"
                        color="primary"
                        track={track}
                    />
                    <GraphEncoder
                        bounds={boundsMarginTop(bounds12)}
                        audioPlugin={synthName}
                        dataId="LAYER2_ENV_AMP_FORM"
                        values={['LAYER2_AMP_MORPH']}
                        // outlineColor="quaternary"
                        // fillColor={rgb(194, 175, 107)}
                        track={track}
                    />
                    <Value
                        {...encBottomValue(enc12)}
                        audioPlugin={synthName}
                        param="LAYER2_AMP_MORPH"
                        track={track}
                        barHeight={1}
                        barColor="primary"
                        alignLeft
                        showLabelOverValue={1}
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