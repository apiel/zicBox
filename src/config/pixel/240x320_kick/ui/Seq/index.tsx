import * as React from '@/libs/react';

import { StepEdit2 } from '@/libs/components/StepEdit2';
import { View } from '@/libs/components/View';
import { rgb } from '@/libs/ui';
import { Common } from '../components/Common';
import { TextGridSeq } from './TextGridSeq';

export type Props = {
    name: string;
};

export function SeqView({ name }: Props) {
    return (
        <View name={name}>
            {Array.from({ length: 32 }, (_, i) => (
                <StepEdit2
                    position={[0, i * 8, 0, 0]}
                    data={`Sequencer ${i}`}
                    group={i}
                    playing_color={rgb(35, 161, 35)}
                    // bar_color={
                    //     // i % 8 == 0 || i % 8 == 1 || i % 8 == 2 || i % 8 == 3
                    //     i % 4 == 0
                    //         ? rgb(104, 149, 160)
                    //         : rgb(197, 216, 178)
                    // }
                />
            ))}

            <TextGridSeq />
            <Common selected={0} hideSpectrogram hideSequencer />
        </View>
    );
}
