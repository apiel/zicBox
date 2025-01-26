import * as React from '@/libs/react';

import { Keymaps } from '@/libs/components/Keymaps';
import { StepEditSample } from '@/libs/components/StepEditSample';
import { Text } from '@/libs/components/Text';
import { TextGrid } from '@/libs/components/TextGrid';
import { View } from '@/libs/components/View';
import { VisibilityContext } from '@/libs/components/VisibilityContext';
import { rgb } from '@/libs/ui';
import { Common } from '../components/Common';
import { KeyInfoPosition, SampleTrack, ScreenWidth } from '../constants';

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

            <TextGrid
                position={KeyInfoPosition}
                rows={[
                    '&icon::toggle::rect &icon::arrowUp::filled ...',
                    '&empty &icon::arrowDown::filled &icon::musicNote::pixelated',
                ]}
            >
                <VisibilityContext index={254} condition="SHOW_WHEN" value={0} />
                <Keymaps
                    keys={[
                        { key: 'e', action: 'contextToggle:254:1:0' },
                        { key: 'w', action: 'incGroup:-1' },
                        { key: 's', action: 'incGroup:+1' },
                    ]}
                />
            </TextGrid>
            <Common selected={1} track={SampleTrack} hideSequencer />
        </View>
    );
}
