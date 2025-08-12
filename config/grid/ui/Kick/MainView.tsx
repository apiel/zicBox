import * as React from '@/libs/react';

import { GraphEncoder } from '@/libs/nativeComponents/GraphEncoder';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { SequencerCard } from '@/libs/nativeComponents/SequencerCard';
import { Value } from '@/libs/nativeComponents/Value';
import { ClipSelection } from '../components/ClipSelection';
import { Drum1 } from '../components/Common';
import { KeysScatter } from '../components/KeysScatter';
import { KeysTracks } from '../components/KeysTracks';
import { Layout } from '../components/Layout';
import { unshiftVisibilityContext } from '../components/ShiftLayout';
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
    encTopValue,
    graphBounds,
    seqCardBounds_small,
} from '../constantsValue';
import { Shift } from './Shift';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function MainView({ name, track, synthName, color }: Props) {
    return (
        <Layout
            viewName={name}
            color={color}
            synthName={synthName}
            title={Drum1}
            content={
                <>
                    <Shift />
                    <KnobValue
                        audioPlugin={synthName}
                        param="CUTOFF"
                        {...enc1}
                        color="secondary"
                        type="STRING"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="RESONANCE"
                        {...enc2}
                        color="secondary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="GAIN_CLIPPING"
                        {...enc3}
                        color="primary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="WAVESHAPE"
                        {...enc4}
                        color="tertiary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <KnobValue
                        audioPlugin={synthName}
                        param="COMPRESS"
                        {...enc7}
                        color="secondary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="DRIVE"
                        {...enc8}
                        color="quaternary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <KnobValue
                        audioPlugin={synthName}
                        param="BASS"
                        {...enc11}
                        color="tertiary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="HIGH_FREQ_BOOST"
                        {...enc12}
                        color="tertiary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <GraphEncoder
                        bounds={graphBounds(bounds5)}
                        audioPlugin={synthName}
                        dataId="WAVEFORM"
                        renderValuesOnTop={false}
                        values={['WAVEFORM_TYPE', 'SHAPE', 'MACRO']}
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />
                    <Value
                        {...encTopValue(enc5)}
                        audioPlugin={synthName}
                        param="WAVEFORM_TYPE"
                        track={track}
                        barHeight={1}
                        // barColor="primary"
                        alignLeft
                        showLabelOverValue={1}
                        visibilityContext={[unshiftVisibilityContext]}
                    />
                    <Value
                        {...encTopValue(enc6)}
                        audioPlugin={synthName}
                        param="SHAPE"
                        track={track}
                        barHeight={1}
                        // barColor="primary"
                        alignLeft
                        showLabelOverValue={1}
                        visibilityContext={[unshiftVisibilityContext]}
                    />
                    <Value
                        {...encBottomValue(enc9)}
                        audioPlugin={synthName}
                        param="MACRO"
                        track={track}
                        barHeight={1}
                        // barColor="primary"
                        alignLeft
                        showLabelOverValue={1}
                        visibilityContext={[unshiftVisibilityContext]}
                    />
                    <Value
                        {...encBottomValue(enc10)}
                        audioPlugin={synthName}
                        param="PITCH"
                        track={track}
                        barHeight={1}
                        // barColor="primary"
                        alignLeft
                        showLabelOverValue={1}
                        visibilityContext={[unshiftVisibilityContext]}
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
