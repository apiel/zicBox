import * as React from '@/libs/react';

import { GraphEncoder } from '@/libs/nativeComponents/GraphEncoder';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { SequencerCard } from '@/libs/nativeComponents/SequencerCard';
import { Value } from '@/libs/nativeComponents/Value';
import { ClipSelection } from '../components/ClipSelection';
import { KeysScatter } from '../components/KeysScatter';
import { KeysTracks } from '../components/KeysTracks';
import { Layout } from '../components/Layout';
import {
    bounds5,
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
    return (
        <Layout
            viewName={name}
            color={color}
            synthName={synthName}
            title={title}
            content={
                <>
                    <KnobValue
                        audioPlugin={synthName}
                        param="FILTER_TYPE"
                        {...enc1}
                        color="secondary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="FILTER_CUTOFF"
                        {...enc2}
                        color="secondary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="FILTER_RESONANCE"
                        {...enc3}
                        color="secondary"
                        track={track}
                    />

                    <KnobValue
                        audioPlugin={synthName}
                        param="PITCH"
                        {...enc4}
                        color="tertiary"
                        track={track}
                    />

                    <GraphEncoder
                        bounds={graphBounds(bounds5, [20, 32], 2, 1)}
                        audioPlugin={synthName}
                        dataId="WAVEFORM"
                        values={['WAVE', 'WAVE_EDIT']}
                        track={track}
                    />
                    <Value
                        {...encBottomValue(enc5)}
                        audioPlugin={synthName}
                        param="WAVE"
                        track={track}
                        barHeight={1}
                        alignLeft
                        showLabelOverValue={0}
                    />
                    <Value
                        {...encBottomValue(enc6)}
                        audioPlugin={synthName}
                        param="WAVE_EDIT"
                        track={track}
                        barHeight={1}
                        alignLeft
                        showLabelOverValue={0}
                    />

                    <KnobValue
                        audioPlugin="TrackFx"
                        param="FX_TYPE"
                        {...enc7}
                        color="quaternary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin="TrackFx"
                        param="FX_AMOUNT"
                        {...enc8}
                        color="quaternary"
                        track={track}
                        valueReplaceTitle
                    />

                    <KnobValue
                        audioPlugin={synthName}
                        param="ATTACK"
                        {...enc9}
                        color="tertiary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="RELEASE"
                        {...enc10}
                        color="tertiary"
                        track={track}
                    />

                    <KnobValue
                        audioPlugin="TrackFx2"
                        param="FX_TYPE"
                        label="FX2 type"
                        {...enc11}
                        color="primary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin="TrackFx2"
                        param="FX_AMOUNT"
                        label="FX2 edit"
                        {...enc12}
                        color="primary"
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
                    <KeysScatter scatterTrack={0} seqTrack={track} />
                </>
            }
        />
    );
}
