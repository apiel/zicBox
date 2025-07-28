import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { SequencerCard } from '@/libs/nativeComponents/SequencerCard';
import { Text } from '@/libs/nativeComponents/Text';
import { rgb } from '@/libs/ui';
import { KeysTracks } from '../components/KeysTracks';
import { Layout } from '../components/Layout';
import {
    enc1,
    enc10,
    enc11,
    enc2,
    enc3,
    enc4,
    enc5,
    enc6,
    enc9,
    seqCardBounds_small,
} from '../constantsValue';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
    title: string;
};

export function Main2View({ name, track, synthName, color, title }: Props) {
    return (
        <Layout
            viewName={name}
            color={color}
            synthName={synthName}
            title={title}
            content={
                <>
                    <Text
                        text="Envelope modulation"
                        bounds={[0, 53, 480, 16]}
                        centered={true}
                        color={rgb(100, 100, 100)}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="LFO_FREQ_MOD"
                        {...enc1}
                        color="secondary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="LFO_WAVE_MOD"
                        {...enc2}
                        color="secondary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="LFO_CUTOFF_MOD"
                        {...enc3}
                        color="secondary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="LFO_RESONANCE_MOD"
                        {...enc4}
                        color="secondary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="LFO_WAVEFORM"
                        {...enc5}
                        color="secondary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="LFO_RATE"
                        {...enc6}
                        color="secondary"
                        track={track}
                    />

                    <Text
                        text="Envelope modulation"
                        bounds={[0, 293, 480, 16]}
                        centered={true}
                        color={rgb(100, 100, 100)}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="CUTOFF_MOD"
                        {...enc9}
                        color="tertiary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="RESONANCE_MOD"
                        {...enc10}
                        color="tertiary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="FREQUENCY_MOD"
                        {...enc11}
                        color="tertiary"
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
