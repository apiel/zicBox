import * as React from '@/libs/react';

import { Keymaps } from '@/libs/nativeComponents/Keymaps';
import { StepEditDrum } from '@/libs/nativeComponents/StepEditDrum';
import { View } from '@/libs/nativeComponents/View';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { rgb } from '@/libs/ui';
import { Common } from '../components/Common';
import { btn1, btn2, btn5, btn6, btn7, btnShift, ColorTrack1, Drum23Track, KeyInfoPosition, ScreenWidth } from '../constants';

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
                        track={Drum23Track}
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
                    'Seq./Kick &icon::arrowDown::filled &icon::musicNote::pixelated',
                    // '!Seq/Kick &icon::arrowDown::filled &icon::musicNote::pixelated',
                ]}
                selectedBackground={ColorTrack1}
                keys={[
                    { key: btn2, action: 'incGroup:-1' },
                    { key: btnShift, action: 'contextToggle:254:1:0' },

                    { key: btn5, action: 'setView:Drum23' },
                    { key: btn6, action: 'incGroup:+1' },
                    { key: btn7, action: 'noteOn:Drum23:60' },
                ]}
                contextValue={0}
            />
            <Common selected={'Kick'} hideSequencer track={Drum23Track}  selectedBackground={ColorTrack1} />
        </View>
    );
}
