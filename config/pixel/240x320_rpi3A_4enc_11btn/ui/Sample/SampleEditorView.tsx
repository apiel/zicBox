import * as React from '@/libs/react';

import { HiddenValue } from '@/libs/nativeComponents/HiddenValue';
import { StepEditSample } from '@/libs/nativeComponents/StepEditSample';
import { Text } from '@/libs/nativeComponents/Text';
import { View } from '@/libs/nativeComponents/View';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { rgb } from '@/libs/ui';
import { Common } from '../components/Common';
import {
    btn1,
    btn7,
    btnDown,
    btnShift,
    btnUp,
    ColorTrack3,
    KeyInfoPosition,
    SampleTrack,
    ScreenWidth,
} from '../constants';

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
                        audioPlugin="SampleSequencer"
                        stepIndex={i}
                        track={SampleTrack}
                        group={i}
                        playingColor={rgb(35, 161, 35)}
                        bgColor={
                            i % 8 == 0 || i % 8 == 1 || i % 8 == 2 || i % 8 == 3
                                ? rgb(42, 54, 56)
                                : 'background'
                        }
                        selectedColor={rgb(76, 94, 97)}
                        keys={[
                            { key: btn1, action: '.toggle' },
                            { key: btn7, action: '.play' },
                        ]}
                    />
                );
            })}

            <HiddenValue
                visibilityContext={[{ index: 254, condition: 'SHOW_WHEN', value: 0 }]}
                keys={[
                    { key: btnShift, action: 'contextToggle:254:1:0' },
                    { key: btnDown, action: 'incGroup:-1' },
                    { key: btnUp, action: 'incGroup:+1' },
                ]}
            />
            <TextGrid
                bounds={KeyInfoPosition}
                rows={[
                    '&icon::toggle::rect &icon::arrowUp::filled ...',
                    '&empty &icon::arrowDown::filled &icon::musicNote::pixelated',
                ]}
                contextValue={0}
            />
            <Common
                selected={'Sampl1'}
                track={SampleTrack}
                hideSequencer
                selectedBackground={ColorTrack3}
            />
        </View>
    );
}
