import * as React from '@/libs/react';

import { HiddenValue } from '@/libs/components/HiddenValue';
import { Keymaps } from '@/libs/components/Keymaps';
import { StepEdit2 } from '@/libs/components/StepEdit2';
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
            <HiddenValue encoder_id={0} inverted>
                <Keymaps
                    keys={[
                        { key: '1', action: 'contextToggleOnRelease:100:1:0' },
                        { key: '2', action: 'contextToggleOnRelease:100:1:0' },
                        { key: '3', action: 'contextToggleOnRelease:100:1:0' },
                        { key: '4', action: 'contextToggleOnRelease:100:1:0' },
                    ]}
                />
            </HiddenValue>

            {Array.from({ length: 32 }, (_, i) => {
                const yy = y + 5;
                y += 8 + (i % 4 == 3 ? 4 : 0);
                return (
                    <StepEdit2
                        position={[0, yy, ScreenWidth, 8]}
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
