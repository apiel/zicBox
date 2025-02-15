import * as React from '@/libs/react';

import { Keymaps } from '@/libs/nativeComponents/Keymaps';
import { StepEditDrum } from '@/libs/nativeComponents/StepEditDrum';
import { View } from '@/libs/nativeComponents/View';
import { VisibilityContext } from '@/libs/nativeComponents/VisibilityContext';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { rgb } from '@/libs/ui';
import { Common } from '../components/Common';
import { BassTrack, btn1, btn2, btn5, btn6, btn7, btnShift, ColorTrack3, Fm1Track, KeyInfoPosition, ScreenWidth } from '../constants';

export type Props = {
    name: string;
};

export function Fm1SeqView({ name }: Props) {
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
                        track={BassTrack}
                        group={i}
                        playing_color={rgb(35, 161, 35)}
                        background_color={
                            i % 8 == 0 || i % 8 == 1 || i % 8 == 2 || i % 8 == 3
                                ? rgb(42, 54, 56)
                                : 'background'
                        }
                        selected_color={rgb(76, 94, 97)}
                    >
                        <Keymaps keys={[{ key: btn1, action: '.toggle' }]} />
                    </StepEditDrum>
                );
            })}

            <TextGrid
                bounds={KeyInfoPosition}
                rows={[
                    '&icon::toggle::rect &icon::arrowUp::filled ...',
                    'Seq./Bass &icon::arrowDown::filled &icon::musicNote::pixelated',
                    // '!Seq/Bass &icon::arrowDown::filled &icon::musicNote::pixelated',
                ]}
                ITEM_BACKGROUND={ColorTrack3}
            >
                <VisibilityContext index={254} condition="SHOW_WHEN" value={0} />
                <Keymaps
                    keys={[
                        { key: btn2, action: 'incGroup:-1' },
                        { key: btnShift, action: 'contextToggle:254:1:0' },

                        { key: btn5, action: 'setView:Fm1' },
                        { key: btn6, action: 'incGroup:+1' },
                        { key: btn7, action: 'noteOn:FmDrum:60' },
                    ]}
                />
            </TextGrid>
            <Common selected={'Fm1'} hideSequencer track={Fm1Track}  selectedBackground={ColorTrack3} />
        </View>
    );
}
