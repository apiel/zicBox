import * as React from '@/libs/react';

import { StepEditDrum } from '@/libs/components/StepEditDrum';
import { View } from '@/libs/components/View';
import { rgb } from '@/libs/ui';
import { Common } from '../components/Common';
import { Drum23Track, ScreenWidth } from '../constants';
import { TextGridSeq } from './TextGridSeq';

export type Props = {
    name: string;
};

export function Drum23SeqView({ name }: Props) {
    let y = 0;
    return (
        <View name={name}>
            {Array.from({ length: 32 }, (_, i) => {
                const yy = y + 5;
                y += 8 + (i % 4 == 3 ? 4 : 0);
                return (
                    <StepEditDrum
                        bounds={[0, yy, ScreenWidth, 8]}
                        data={`Sequencer ${i}`}
                        group={i}
                        playing_color={rgb(35, 161, 35)}
                        background_color={
                            i % 8 == 0 || i % 8 == 1 || i % 8 == 2 || i % 8 == 3
                                ? rgb(42, 54, 56)
                                : 'background'
                        }
                        selected_color={rgb(76, 94, 97)}
                    />
                );
            })}

            <TextGridSeq />
            <Common selected={0} hideSequencer track={Drum23Track} />
        </View>
    );
}
