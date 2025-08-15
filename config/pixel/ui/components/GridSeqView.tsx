import * as React from '@/libs/react';

import { SequencerCard } from '@/libs/nativeComponents/SequencerCard';
import { SequencerValue } from '@/libs/nativeComponents/SequencerValue';
import { A1, A2, A3, A4, A5, A6, A7, A8, D10, ScreenHeight, ScreenWidth } from '../constants';
import { enc1Seq, enc2Seq, enc3Seq, enc4Seq, enc5Seq, enc6Seq, enc7Seq } from '../constantsValue';
import { Layout } from './Layout';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
    contextId: number;
    title: string;
    includeLength?: boolean;
};

export function GridSeqView({
    name,
    track,
    synthName,
    color,
    contextId,
    title,
    includeLength,
}: Props) {
    return (
        <Layout
            viewName={name}
            color={color}
            synthName={synthName}
            title={title}
            content={
                <>
                    <SequencerValue
                        {...enc1Seq}
                        audioPlugin={`Sequencer`}
                        track={track}
                        contextId={contextId}
                        fontValue={'PoppinsLight_24'}
                        fontLabel={'PoppinsLight_12'}
                        type={'STEP_SELECTION'}
                    />
                    <SequencerValue
                        {...enc2Seq}
                        audioPlugin={`Sequencer`}
                        track={track}
                        contextId={contextId}
                        fontValue={'PoppinsLight_24'}
                        fontLabel={'PoppinsLight_12'}
                        type={'STEP_TOGGLE'}
                        barColor="primary"
                    />
                    <SequencerValue
                        {...enc3Seq}
                        audioPlugin={`Sequencer`}
                        track={track}
                        contextId={contextId}
                        fontValue={'PoppinsLight_24'}
                        fontLabel={'PoppinsLight_12'}
                        type={'STEP_NOTE'}
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
                    />
                    <SequencerValue
                        {...enc5Seq}
                        audioPlugin={`Sequencer`}
                        track={track}
                        contextId={contextId}
                        fontValue={'PoppinsLight_24'}
                        fontLabel={'PoppinsLight_12'}
                        type={'STEP_CONDITION'}
                    />
                    <SequencerValue
                        {...enc6Seq}
                        audioPlugin={`Sequencer`}
                        track={track}
                        contextId={contextId}
                        fontValue={'PoppinsLight_24'}
                        fontLabel={'PoppinsLight_12'}
                        type={'STEP_MOTION'}
                    />
                    {includeLength && (
                        <SequencerValue
                            {...enc7Seq}
                            audioPlugin={`Sequencer`}
                            track={track}
                            contextId={contextId}
                            fontValue={'PoppinsLight_24'}
                            fontLabel={'PoppinsLight_12'}
                            type={'STEP_LENGTH'}
                            barColor="primary"
                        />
                    )}
                       
                    <SequencerCard
                        bounds={[ScreenWidth - 65, 5, 60, ScreenHeight]}
                        audioPlugin={`Sequencer`}
                        track={track}
                        contextId={contextId}
                        rowsSelection={2}
                        stepPerRow={4}
                        // rowsSelectionColor={"#28595f"}
                        gridKeys={[A1, A2, A3, A4, A5, A6, A7, A8]}
                        keys={[{ key: D10, action: '.scroll' }]}
                    />
                </>
            }
        />
    );
}
