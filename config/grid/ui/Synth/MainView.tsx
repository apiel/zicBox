import * as React from '@/libs/react';

import { GraphEncoder } from '@/libs/nativeComponents/GraphEncoder';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { SequencerCard } from '@/libs/nativeComponents/SequencerCard';
import { Value } from '@/libs/nativeComponents/Value';
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
    seqCardBounds_small
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
                        color="primary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="FILTER_RESONANCE"
                        {...enc3}
                        color="primary"
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
                        fontLabel="PoppinsLight_6"
                        barHeight={1}
                        alignLeft
                        showLabelOverValue={0}
                    />
                    <Value
                        {...encBottomValue(enc6)}
                        audioPlugin={synthName}
                        param="WAVE_EDIT"
                        track={track}
                        fontLabel="PoppinsLight_6"
                        barHeight={1}
                        alignLeft
                        showLabelOverValue={0}
                    />

                    <KnobValue
                        audioPlugin="TrackFx"
                        param="FX_TYPE"
                        {...enc7}
                        color="tertiary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin="TrackFx"
                        param="FX_AMOUNT"
                        {...enc8}
                        color="primary"
                        track={track}
                        valueReplaceTitle
                    />

                    <KnobValue
                        audioPlugin={synthName}
                        param="LFO_FREQ_MOD"
                        {...enc9}
                        color="tertiary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="LFO_WAVE_MOD"
                        {...enc10}
                        color="primary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="LFO_CUTOFF_MOD"
                        {...enc11}
                        color="quaternary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="LFO_RESONANCE_MOD"
                        {...enc12}
                        color="secondary"
                        track={track}
                    />

                    <SequencerCard
                        bounds={seqCardBounds_small}
                        audioPlugin={`Sequencer`}
                        track={track}
                        // contextId={seqContextDrum1}
                    />

                    <KeysTracks synthName={synthName} viewName={name} />
                </>
            }
        />
    );
}
