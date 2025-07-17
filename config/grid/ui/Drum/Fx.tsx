import * as React from '@/libs/react';

import { SequencerCard } from '@/libs/nativeComponents/SequencerCard';
import { SequencerValue } from '@/libs/nativeComponents/SequencerValue';
import { enc1, enc2 } from '../constantsValue';
import { DrumLayout } from './Layout';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function FxView({ name, track, synthName, color }: Props) {
    return (
        <DrumLayout
            viewName={name}
            color={color}
            synthName={synthName}
            content={
                <>
                    <SequencerValue
                        {...enc1}
                        audioPlugin={`Sequencer`}
                        track={track}
                        contextId={50}
                        fontValue={'PoppinsLight_24'}
                        fontLabel={'PoppinsLight_12'}
                        type={'STEP_SELECTION'}
                    />
                    <SequencerValue
                        {...enc2}
                        audioPlugin={`Sequencer`}
                        track={track}
                        contextId={50}
                        fontValue={'PoppinsLight_24'}
                        fontLabel={'PoppinsLight_12'}
                        type={'STEP_TOGGLE'}
                        barColor='primary'
                    />
                    <SequencerCard
                        bounds={[130, 380, 200, 100]}
                        audioPlugin={`Sequencer`}
                        track={track}
                        contextId={50}
                        rowsSelection={2}
                    />
                </>
            }
        />
    );
}
