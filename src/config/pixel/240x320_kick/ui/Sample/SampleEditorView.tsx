import * as React from '@/libs/react';

import { Keymaps } from '@/libs/nativeComponents/Keymaps';
import { StepEditSample } from '@/libs/nativeComponents/StepEditSample';
import { Text } from '@/libs/nativeComponents/Text';
import { View } from '@/libs/nativeComponents/View';
import { VisibilityContext } from '@/libs/nativeComponents/VisibilityContext';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { rgb } from '@/libs/ui';
import { Common } from '../components/Common';
import { ColorTrack3, KeyInfoPosition, SampleTrack, ScreenWidth } from '../constants';

export type Props = {
    name: string;
};

export function SampleEditorView({ name }: Props) {
    let y = 0;
    return (
        <View name={name}>
            <Text text="Vel." bounds={[0, 0, 25, 9]} />
            <Text text="Sample" bounds={[60, 0, 100, 9]} />
            <Text text="Start" bounds={[ScreenWidth - 84, 0, 50, 9]} />
            <Text text="End" bounds={[ScreenWidth - 38, 0, 50, 9]} />
            {Array.from({ length: 32 }, (_, i) => {
                const yy = y + 12;
                y += 8 + (i % 4 == 3 ? 3 : 0);
                return (
                    <StepEditSample
                        bounds={[0, yy, ScreenWidth, 8]}
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
                    >
                        <Keymaps
                            keys={[
                                { key: 'q', action: '.toggle' },
                                { key: 'd', action: '.play' },
                            ]}
                        />
                    </StepEditSample>
                );
            })}

            <TextGrid
                bounds={KeyInfoPosition}
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
            <Common selected={1} track={SampleTrack} hideSequencer selectedBackground={ColorTrack3} />
        </View>
    );
}
