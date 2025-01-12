import * as React from '@/libs/react';

import { StepEdit2 } from '@/libs/components/StepEdit2';
import { View } from '@/libs/components/View';
import { rgb } from '@/libs/ui';
import { Common } from '../components/Common';
import { ScreenWidth } from '../constants';
import { TextGridSeq } from './TextGridSeq';

export type Props = {
    name: string;
};

export function SeqView({ name }: Props) {
    let y = 0;
    return (
        <View name={name}>
            {Array.from({ length: 32 }, (_, i) => {
                const yy = y;
                y += 8 + (i % 4 == 3 ? 3 : 0);
                return (
                    <StepEdit2
                        position={[0, yy, ScreenWidth, 8]}
                        data={`Sequencer ${i}`}
                        group={i}
                        playing_color={rgb(35, 161, 35)}
                        background_color={
                            i % 8 == 0 || i % 8 == 1 || i % 8 == 2 || i % 8 == 3
                                ? rgb(42, 54, 56)
                                : "background"
                        }
                    />
                );
            })}

            {/* {Array.from({ length: 7 }, (_, i) => (
                <Rect position={[0, ((i + 1) * 35) - 2, ScreenWidth, 1]} color={rgb(58, 74, 78)} />
            ))} */}

            <TextGridSeq />
            <Common selected={0} hideSpectrogram hideSequencer />
        </View>
    );
}
