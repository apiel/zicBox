import * as React from '@/libs/react';

import { GraphEncoder } from '@/libs/nativeComponents/GraphEncoder';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { SequencerCard } from '@/libs/nativeComponents/SequencerCard';
import { StringVal } from '@/libs/nativeComponents/StringVal';
import { Value } from '@/libs/nativeComponents/Value';
import { rgb } from '@/libs/ui';
import { ClipSelection } from '../components/ClipSelection';
import { KeysScatter } from '../components/KeysScatter';
import { KeysTracks } from '../components/KeysTracks';
import { Layout } from '../components/Layout';
import { ShiftLayout, shiftVisibilityContext, unshiftVisibilityContext } from '../components/ShiftLayout';
import {
    bounds2,
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
                    <ShiftLayout
                        track={track}
                        content={
                            <KnobValue
                                audioPlugin={synthName}
                                param="ENGINE"
                                {...enc2}
                                color="secondary"
                                track={track}
                                visibilityContext={[shiftVisibilityContext]}
                            />
                        }
                    />

                    <StringVal audioPlugin={synthName} param="ENGINE" bounds={[280, 22, 100, 20]} />

                    <KnobValue
                        audioPlugin={synthName}
                        param="DURATION"
                        {...enc1}
                        color="secondary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <GraphEncoder
                        bounds={boundsMarginTop(bounds2)}
                        audioPlugin={synthName}
                        dataId="ENV_AMP_FORM"
                        values={['AMP_MORPH']}
                        outlineColor="quaternary"
                        fillColor={rgb(194, 175, 107)}
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />
                    <Value
                        {...encBottomValue(enc2)}
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
                        param="VAL_3"
                        {...enc3}
                        color="tertiary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <KnobValue
                        audioPlugin={synthName}
                        param="VAL_4"
                        {...enc4}
                        color="primary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <KnobValue
                        audioPlugin={synthName}
                        param="VAL_5"
                        {...enc5}
                        color="secondary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <KnobValue
                        audioPlugin={synthName}
                        param="VAL_6"
                        {...enc6}
                        color="quaternary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <KnobValue
                        audioPlugin={synthName}
                        param="VAL_7"
                        {...enc7}
                        color="tertiary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <KnobValue
                        audioPlugin={synthName}
                        param="VAL_8"
                        {...enc8}
                        color="primary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <KnobValue
                        audioPlugin={synthName}
                        param="VAL_9"
                        {...enc9}
                        color="secondary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <KnobValue
                        audioPlugin={synthName}
                        param="VAL_10"
                        {...enc10}
                        color="quaternary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <KnobValue
                        audioPlugin={synthName}
                        param="VAL_11"
                        {...enc11}
                        color="tertiary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <KnobValue
                        audioPlugin={synthName}
                        param="VAL_12"
                        {...enc12}
                        color="primary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <SequencerCard
                        bounds={seqCardBounds_small}
                        audioPlugin={`Sequencer`}
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <ClipSelection track={track} color={color} />
                    <KeysTracks synthName={synthName} viewName={name} />
                    <KeysScatter scatterTrack={0} seqTrack={track} />
                </>
            }
        />
    );
}
