import * as React from '@/libs/react';

import { StepEditSample } from '@/libs/components/StepEditSample';
import { Text } from '@/libs/components/Text';
import { View } from '@/libs/components/View';
import { rgb } from '@/libs/ui';
import { Common } from '../components/Common';
import { TextGridSel } from '../components/TextGridSel';
import { SampleTrack, ScreenWidth } from '../constants';

export type Props = {
    name: string;
};

export function SampleEditorView({ name }: Props) {
    let y = 0;
    return (
        <View name={name}>
            <Text text="Vel." position={[0, 0, 25, 9]} />
            <Text text="Sample" position={[60, 0, 100, 9]} />
            <Text text="Start" position={[ScreenWidth - 84, 0, 50, 9]} />
            <Text text="End" position={[ScreenWidth - 38, 0, 50, 9]} />
            {Array.from({ length: 32 }, (_, i) => {
                const yy = y + 12;
                y += 8 + (i % 4 == 3 ? 3 : 0);
                return (
                    <StepEditSample
                        position={[0, yy, ScreenWidth, 8]}
                        data={`SampleSequencer ${i}`}
                        track={SampleTrack}
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

            <TextGridSel
                items={['Editor', 'Sample', '...', '&empty', 'Seq.', '&icon::musicNote::pixelated']}
                keys={
                    [
                        // { key: 'q', action: 'setView:SampleEditor' },
                        // {
                        //     key: 'w',
                        //     action: viewName === 'Sample' ? 'setView:Sample2' : 'setView:Sample',
                        // },
                        // { key: 'e', action: 'contextToggle:254:1:0' },
                        // // {
                        // //     key: 'a',
                        // //     action: viewName === 'HiHat' ? 'setView:HiHat2' : 'setView:HiHat',
                        // // },
                        // { key: 's', action: 'setView:DrumsSeq' },
                        // { key: 'd', action: `noteOn:${target}:60` },
                    ]
                }
                selected={0}
                contextValue={0}
            />
            <Common selected={1} track={SampleTrack} hideSequencer />
        </View>
    );
}
