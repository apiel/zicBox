import * as React from '@/libs/react';

import { StepEdit2 } from '@/libs/components/StepEdit2';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { TextGridSeq } from './TextGridSeq';

export type Props = {
    name: string;
};

export function SeqView({ name }: Props) {
    return (
        <View name={name}>
            {Array.from({ length: 32 }, (_, i) => (
                <StepEdit2 position={[0, i * 8, 0, 0]} data={`Sequencer GET_STEP ${i}`} group={i} />
            ))}

            <TextGridSeq />
            <Common selected={0} hideSpectrogram hideSequencer />
        </View>
    );
}
