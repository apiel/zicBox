import * as React from '@/libs/react';

import { VisibilityContext } from '@/libs/nativeComponents/component';
import { Rect } from '@/libs/nativeComponents/Rect';
import { SequencerCard } from '@/libs/nativeComponents/SequencerCard';
import { SequencerValue } from '@/libs/nativeComponents/SequencerValue';
import { Text } from '@/libs/nativeComponents/Text';
import { rgb } from '@/libs/ui';
import {
    A1,
    A2,
    A3,
    A4,
    B1,
    B2,
    B3,
    B4,
    C1,
    C2,
    C4,
    ScreenHeight,
    ScreenWidth,
    seqContext,
    W1_4,
    W2_4,
    W3_4
} from '../constants';
import {
    enc1Seq,
    enc2Seq,
    enc3Seq,
    enc4Seq,
    enc5Seq,
    enc6Seq,
    enc7Seq,
    seqTop,
} from '../constantsValue';
import { Layout } from './Layout';
import { shiftVisibilityContext, unshiftVisibilityContext } from './ShiftLayout';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
    contextId: number;
    title: string;
    includeLength?: boolean;
};

export function SeqView({ name, track, synthName, color, contextId, title, includeLength }: Props) {
    // const row1: VisibilityContext[] = [
    //     {
    //         index: seqContext,
    //         value: 0,
    //         condition: 'SHOW_WHEN',
    //     },
    // ];
    // const row2: VisibilityContext[] = [
    //     {
    //         index: seqContext,
    //         value: 1,
    //         condition: 'SHOW_WHEN',
    //     },
    // ];
    const row1: VisibilityContext[] = [unshiftVisibilityContext];
    const row2: VisibilityContext[] = [shiftVisibilityContext];
    const colorOff = rgb(90, 90, 90);
    const menuTextColor = rgb(75, 75, 75);
    return (
        <Layout
            viewName={name}
            color={color}
            synthName={synthName}
            title={title}
            content={
                <>
                    <Rect
                        bounds={[0, seqTop - 10, 2, 60]}
                        color={rgb(100, 100, 100)}
                        visibilityContext={row1}
                    />
                    <SequencerValue
                        {...enc1Seq}
                        audioPlugin={`Sequencer`}
                        track={track}
                        contextId={contextId}
                        fontValue={'PoppinsLight_24'}
                        fontLabel={'PoppinsLight_12'}
                        type={'STEP_SELECTION'}
                        visibilityContext={row1}
                    />
                    <SequencerValue
                        {...enc2Seq}
                        audioPlugin={`Sequencer`}
                        track={track}
                        contextId={contextId}
                        fontValue={'PoppinsLight_24'}
                        fontLabel={'PoppinsLight_12'}
                        type={'STEP_LENGTH_AND_TOGGLE'}
                        maxSteps={includeLength ? 64 : 0} // we set 0 so it is either 0 or 1..
                        barColor="primary"
                        visibilityContext={row1}
                    />
                    <SequencerValue
                        {...enc3Seq}
                        audioPlugin={`Sequencer`}
                        track={track}
                        contextId={contextId}
                        fontValue={'PoppinsLight_24'}
                        fontLabel={'PoppinsLight_12'}
                        type={'STEP_NOTE'}
                        visibilityContext={row1}
                    />
                    <SequencerValue
                        {...enc4Seq}
                        bounds={[
                            enc4Seq.bounds[0] + 10,
                            enc4Seq.bounds[1],
                            enc4Seq.bounds[2] - 20,
                            enc4Seq.bounds[3],
                        ]}
                        audioPlugin={`Sequencer`}
                        track={track}
                        contextId={contextId}
                        fontValue={'PoppinsLight_24'}
                        fontLabel={'PoppinsLight_12'}
                        type={'STEP_VELOCITY'}
                        barColor="primary"
                        visibilityContext={row1}
                    />

                    <Rect
                        bounds={[0, seqTop + 48, 2, 60]}
                        color="background"
                        visibilityContext={row1}
                    />
                    <SequencerValue
                        {...enc5Seq}
                        audioPlugin={`Sequencer`}
                        track={track}
                        contextId={contextId}
                        fontValue={'PoppinsLight_16'}
                        fontLabel={'PoppinsLight_12'}
                        type={'STEP_CONDITION'}
                        labelColor={colorOff}
                        valueColor={colorOff}
                        encoderId={-1}
                        visibilityContext={row1}
                    />
                    <SequencerValue
                        {...enc6Seq}
                        audioPlugin={`Sequencer`}
                        track={track}
                        contextId={contextId}
                        fontValue={'PoppinsLight_16'}
                        fontLabel={'PoppinsLight_12'}
                        type={'STEP_MOTION'}
                        labelColor={colorOff}
                        valueColor={colorOff}
                        encoderId={-1}
                        visibilityContext={row1}
                    />

                    <Rect
                        bounds={[0, seqTop - 10, 2, 60]}
                        color="background"
                        visibilityContext={row2}
                    />
                    <SequencerValue
                        {...enc1Seq}
                        audioPlugin={`Sequencer`}
                        track={track}
                        contextId={contextId}
                        fontValue={'PoppinsLight_24'}
                        fontLabel={'PoppinsLight_12'}
                        type={'STEP_SELECTION'}
                        visibilityContext={row2}
                        labelColor={colorOff}
                        valueColor={colorOff}
                        encoderId={-1}
                    />
                    <SequencerValue
                        {...enc2Seq}
                        audioPlugin={`Sequencer`}
                        track={track}
                        contextId={contextId}
                        fontValue={'PoppinsLight_24'}
                        fontLabel={'PoppinsLight_12'}
                        type={'STEP_TOGGLE'}
                        barColor={colorOff}
                        visibilityContext={row2}
                        labelColor={colorOff}
                        valueColor={colorOff}
                        encoderId={-1}
                    />
                    <SequencerValue
                        {...enc3Seq}
                        audioPlugin={`Sequencer`}
                        track={track}
                        contextId={contextId}
                        fontValue={'PoppinsLight_24'}
                        fontLabel={'PoppinsLight_12'}
                        type={'STEP_NOTE'}
                        visibilityContext={row2}
                        labelColor={colorOff}
                        valueColor={colorOff}
                        encoderId={-1}
                    />
                    <SequencerValue
                        {...enc4Seq}
                        bounds={[
                            enc4Seq.bounds[0] + 10,
                            enc4Seq.bounds[1],
                            enc4Seq.bounds[2] - 20,
                            enc4Seq.bounds[3],
                        ]}
                        audioPlugin={`Sequencer`}
                        track={track}
                        contextId={contextId}
                        fontValue={'PoppinsLight_24'}
                        fontLabel={'PoppinsLight_12'}
                        type={'STEP_VELOCITY'}
                        barColor={colorOff}
                        visibilityContext={row2}
                        labelColor={colorOff}
                        valueColor={colorOff}
                        encoderId={-1}
                    />

                    <Rect
                        bounds={[0, seqTop + 48, 2, 60]}
                        color={rgb(100, 100, 100)}
                        visibilityContext={row2}
                    />
                    <SequencerValue
                        {...enc5Seq}
                        audioPlugin={`Sequencer`}
                        track={track}
                        contextId={contextId}
                        fontValue={'PoppinsLight_16'}
                        fontLabel={'PoppinsLight_12'}
                        type={'STEP_CONDITION'}
                        encoderId={enc1Seq.encoderId}
                        visibilityContext={row2}
                    />
                    <SequencerValue
                        {...enc6Seq}
                        audioPlugin={`Sequencer`}
                        track={track}
                        contextId={contextId}
                        fontValue={'PoppinsLight_16'}
                        fontLabel={'PoppinsLight_12'}
                        type={'STEP_MOTION'}
                        encoderId={enc2Seq.encoderId}
                        visibilityContext={row2}
                    />
                    {includeLength && (
                        <SequencerValue
                            {...enc7Seq}
                            audioPlugin={`Sequencer`}
                            track={track}
                            contextId={contextId}
                            fontValue={'PoppinsLight_16'}
                            fontLabel={'PoppinsLight_12'}
                            type={'STEP_LENGTH'}
                            barColor="primary"
                            encoderId={enc3Seq.encoderId}
                            visibilityContext={row2}
                        />
                    )}

                    {/* <Text
                        text="Scroll"
                        bounds={[0, ScreenHeight - 20, W1_4, 16]}
                        centered={true}
                        color={menuTextColor}
                    />
                    <Text
                        text="Toggle knobs"
                        bounds={[W1_4, ScreenHeight - 20, W1_4, 16]}
                        centered={true}
                        color={menuTextColor}
                    /> */}
                    <Text
                        text="&icon::arrowUp::filled"
                        bounds={[0, ScreenHeight - 18, W1_4, 16]}
                        centered={true}
                        color={menuTextColor}
                    />
                    <Text
                        text="&icon::arrowDown::filled"
                        bounds={[W1_4 - 10, ScreenHeight - 18, W1_4, 16]}
                        centered={true}
                        color={menuTextColor}
                    />
                    <Text
                        text="Exit"
                        bounds={[W2_4 - 20, ScreenHeight - 20, W1_4, 16]}
                        centered={true}
                        color={menuTextColor}
                    />
                    <Text
                        text="Shift"
                        bounds={[W3_4 - 30, ScreenHeight - 20, W1_4, 16]}
                        centered={true}
                        color={menuTextColor}
                    />

                    <SequencerCard
                        bounds={[ScreenWidth - 45, 5, 40, ScreenHeight]}
                        audioPlugin={`Sequencer`}
                        track={track}
                        contextId={contextId}
                        rowsSelection={2}
                        stepPerRow={4}
                        // rowsSelectionColor={"#28595f"}
                        gridKeys={[A1, A2, A3, A4, B1, B2, B3, B4]}
                        keys={[
                            // { key: C1, action: '.scroll' }, // , context: { id: shiftContext, value: 0 }
                            // { key: C2, action: `contextToggleOnRelease:${seqContext}:1:0` },
                            { key: C1, action: '.scroll:-1' },
                            { key: C2, action: '.scroll' },
                            {
                                key: C4,
                                action: `contextToggleOnRelease:${seqContext}:1:0`,
                                multipleKeyHandler: true,
                            },
                        ]}
                    />
                </>
            }
        />
    );
}
