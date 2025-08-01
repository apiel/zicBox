import * as React from '@/libs/react';

import { SequencerCard } from '@/libs/nativeComponents/SequencerCard';
import { SequencerValue } from '@/libs/nativeComponents/SequencerValue';
import { A1, A10, A2, A3, A4, A5, A6, A7, A8, A9, B1, B10, B2, B3, B4, B5, B6, B7, B8, B9, C1, C10, C2, C3, C4, C5, C6, C7, C8, C9, D1, D10, D2, D3, D4, D5, D6, D7, D8, D9 } from '../constants';
import { enc1, enc2, enc3, enc4, enc5, enc6, enc7 } from '../constantsValue';
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
                        {...enc1}
                        audioPlugin={`Sequencer`}
                        track={track}
                        contextId={contextId}
                        fontValue={'PoppinsLight_24'}
                        fontLabel={'PoppinsLight_12'}
                        type={'STEP_SELECTION'}
                    />
                    <SequencerValue
                        {...enc2}
                        audioPlugin={`Sequencer`}
                        track={track}
                        contextId={contextId}
                        fontValue={'PoppinsLight_24'}
                        fontLabel={'PoppinsLight_12'}
                        type={'STEP_TOGGLE'}
                        barColor="primary"
                    />
                    <SequencerValue
                        {...enc3}
                        audioPlugin={`Sequencer`}
                        track={track}
                        contextId={contextId}
                        fontValue={'PoppinsLight_24'}
                        fontLabel={'PoppinsLight_12'}
                        type={'STEP_NOTE'}
                    />
                    <SequencerValue
                        {...enc4}
                        bounds={[
                            enc4.bounds[0] + 10,
                            enc4.bounds[1],
                            enc4.bounds[2] - 20,
                            enc4.bounds[3],
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
                        {...enc5}
                        audioPlugin={`Sequencer`}
                        track={track}
                        contextId={contextId}
                        fontValue={'PoppinsLight_24'}
                        fontLabel={'PoppinsLight_12'}
                        type={'STEP_CONDITION'}
                    />
                    <SequencerValue
                        {...enc6}
                        audioPlugin={`Sequencer`}
                        track={track}
                        contextId={contextId}
                        fontValue={'PoppinsLight_24'}
                        fontLabel={'PoppinsLight_12'}
                        type={'STEP_MOTION'}
                    />
                    {includeLength && (
                        <SequencerValue
                            {...enc7}
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
                        bounds={[10, 310, 460, 320]}
                        audioPlugin={`Sequencer`}
                        track={track}
                        contextId={contextId}
                        rowsSelection={4}
                        // rowsSelectionColor={"#28595f"}
                        gridKeys={[
                            A1, A2, A3, A4, A5, A6, A7, A8,
                            B1, B2, B3, B4, B5, B6, B7, B8,
                            C1, C2, C3, C4, C5, C6, C7, C8,
                            D1, D2, D3, D4, D5, D6, D7, D8,
                        ]}
                        controllerColors={[{ controller: 'Default', colors:[
                            { key: A9, color: "#0000000" },
                            { key: B9, color: "#0000000" },
                            { key: C9, color: "#0000000" },
                            { key: D9, color: "#0000000" },
            
                            { key: A10, color: "#0000000" },
                            { key: B10, color: "#0000000" },
                            { key: C10, color: "#0000000" },
                            { key: D10, color: "#021014" },
                        ] }]}
                        keys={[
                            { key: D10, action: '.scroll' },
                        ]}
                    />
                </>
            }
        />
    );
}
