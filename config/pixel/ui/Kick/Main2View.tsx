import * as React from '@/libs/react';

import { GraphEncoder } from '@/libs/nativeComponents/GraphEncoder';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { MacroEnvelop } from '@/libs/nativeComponents/MacroEnvelop';
import { SequencerCard } from '@/libs/nativeComponents/SequencerCard';
import { Value } from '@/libs/nativeComponents/Value';
import { rgb } from '@/libs/ui';
import { ClipSelection } from '../components/ClipSelection';
import { Drum1 } from '../components/Common';
import { KeysScatter } from '../components/KeysScatter';
import { KeysTracks } from '../components/KeysTracks';
import { Layout } from '../components/Layout';
import { unshiftVisibilityContext } from '../components/ShiftLayout';
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
    seqCardBounds_small,
} from '../constantsValue';
import { Shift } from './Shift';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function Main2View({ name, track, synthName, color }: Props) {
    return (
        <Layout
            viewName={name}
            color={color}
            synthName={synthName}
            title={Drum1}
            content={
                <>
                    <Shift />
                    <MacroEnvelop
                        bounds={graphBounds(bounds1, 15)}
                        audioPlugin={synthName}
                        envelopDataId="ENV_FREQ2"
                        track={track}
                        encoders={[enc1.encoderId, enc2.encoderId, enc5.encoderId, enc6.encoderId]}
                        title="Freq. Env."
                        visibilityContext={[unshiftVisibilityContext]}
                    />
                    <KnobValue
                        {...enc9}
                        audioPlugin={synthName}
                        param="DURATION"
                        color="quaternary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />
                    <GraphEncoder
                        bounds={boundsMarginTop(bounds10)}
                        audioPlugin={synthName}
                        dataId="ENV_AMP_FORM"
                        values={['AMP_MORPH']}
                        outlineColor="quaternary"
                        fillColor={rgb(194, 175, 107)}
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
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
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <KnobValue
                        audioPlugin={synthName}
                        param="OSC2"
                        {...enc3}
                        color="secondary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="OSC2_FREQ"
                        {...enc4}
                        color="quaternary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="LAYER2_CUTOFF"
                        {...enc7}
                        color="tertiary"
                        type="STRING"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="LAYER2_RESONANCE"
                        {...enc8}
                        color="tertiary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <KnobValue
                        {...enc11}
                        audioPlugin={synthName}
                        param="LAYER2_DURATION"
                        color="primary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />
                    <GraphEncoder
                        bounds={boundsMarginTop(bounds12)}
                        audioPlugin={synthName}
                        dataId="LAYER2_ENV_AMP_FORM"
                        values={['LAYER2_AMP_MORPH']}
                        // outlineColor="quaternary"
                        // fillColor={rgb(194, 175, 107)}
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
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
