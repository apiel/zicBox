import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { Sample } from '@/libs/nativeComponents/Sample';
import { SequencerCard } from '@/libs/nativeComponents/SequencerCard';
import { Value } from '@/libs/nativeComponents/Value';
import { ClipSelection } from '../components/ClipSelection';
import { KeysTracks } from '../components/KeysTracks';
import { Layout } from '../components/Layout';
import {
    bounds1,
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
    encTopValue,
    graphBounds,
    seqCardBounds_small,
} from '../constantsValue';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
    title: string;
};

export function MainView({ name, track, synthName, color, title }: Props) {
    // const enc3Bounds = encTopValue(enc3);
    // enc3Bounds.bounds[2] = W1_2 - 2;
    return (
        <Layout
            viewName={name}
            color={color}
            synthName={synthName}
            title={title}
            content={
                <>
                    <Value
                        {...encTopValue(enc1)}
                        audioPlugin={synthName}
                        param="START"
                        track={track}
                        barHeight={1}
                        // barColor="primary"
                        alignLeft
                        showLabelOverValue={1}
                    />
                    <Value
                        {...encTopValue(enc2)}
                        audioPlugin={synthName}
                        param="END"
                        track={track}
                        barHeight={1}
                        // barColor="primary"
                        alignLeft
                        showLabelOverValue={1}
                    />
                    {/* <Value
                        {...enc3Bounds}
                        audioPlugin={synthName}
                        param="BROWSER"
                        track={track}
                        barHeight={1}
                        // barColor="primary"
                        alignLeft
                        showLabelOverValue={1}
                    /> */}
                    <Value
                        {...encTopValue(enc3)}
                        audioPlugin="Filter"
                        param="CUTOFF"
                        track={track}
                        barHeight={1}
                        // barColor="primary"
                        alignLeft
                        showLabelOverValue={1}
                    />
                    <Value
                        {...encTopValue(enc4)}
                        audioPlugin="Filter"
                        param="RESONANCE"
                        track={track}
                        barHeight={1}
                        // barColor="primary"
                        alignLeft
                        showLabelOverValue={1}
                    />
                    <Sample
                        bounds={graphBounds(bounds1, 60, 4)}
                        audioPlugin={synthName}
                        track={track}
                    />
                    <Value
                        {...encBottomValue(enc5)}
                        audioPlugin={synthName}
                        param="LOOP_POSITION"
                        track={track}
                        barHeight={1}
                        // barColor="primary"
                        alignLeft
                        showLabelOverValue={1}
                    />
                    <Value
                        {...encBottomValue(enc6)}
                        audioPlugin={synthName}
                        param="LOOP_LENGTH"
                        track={track}
                        barHeight={1}
                        // barColor="primary"
                        alignLeft
                        showLabelOverValue={1}
                    />
                    <Value
                        {...encBottomValue(enc7)}
                        audioPlugin={synthName}
                        param="LOOP_RELEASE"
                        track={track}
                        barHeight={1}
                        // barColor="primary"
                        alignLeft
                        showLabelOverValue={1}
                    />
                    <Value
                        {...encBottomValue(enc8)}
                        audioPlugin={synthName}
                        param="BROWSER"
                        fontValue="PoppinsLight_8"
                        track={track}
                        barHeight={1}
                        // barColor="primary"
                        alignLeft
                        showLabelOverValue={1}
                    />

                    {/* <KnobValue
                        audioPlugin="Filter"
                        param="CUTOFF"
                        type="STRING"
                        {...enc9}
                        color="secondary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin="Filter"
                        param="RESONANCE"
                        {...enc10}
                        color="secondary"
                        track={track}
                    /> */}
                    <KnobValue
                        audioPlugin="TrackFx"
                        param="FX_TYPE"
                        {...enc9}
                        color="secondary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin="TrackFx"
                        param="FX_AMOUNT"
                        {...enc10}
                        color="secondary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin="TrackFx2"
                        param="FX_TYPE"
                        label="FX2 type"
                        {...enc11}
                        color="quaternary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin="TrackFx2"
                        param="FX_AMOUNT"
                        label="FX2 edit"
                        {...enc12}
                        color="quaternary"
                        track={track}
                        valueReplaceTitle
                    />

                    <SequencerCard
                        bounds={seqCardBounds_small}
                        audioPlugin={`Sequencer`}
                        track={track}
                        // contextId={seqContextDrum1}
                    />

                    <ClipSelection track={track} color={color} />
                    <KeysTracks synthName={synthName} viewName={name} />
                </>
            }
        />
    );
}
