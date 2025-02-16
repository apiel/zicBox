import * as React from '@/libs/react';

import { Keymaps } from '@/libs/nativeComponents/Keymaps';
import { StepEditDrum } from '@/libs/nativeComponents/StepEditDrum';
import { View } from '@/libs/nativeComponents/View';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { rgb } from '@/libs/ui';
import { Common } from '../components/Common';
import { BassTrack, ColorTrack2, KeyInfoPosition, ScreenWidth } from '../constants';

export type Props = {
    name: string;
};

export function BassSeqView({ name }: Props) {
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
                        <Keymaps keys={[{ key: 'q', action: '.toggle' }]} />
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
                selectedBackground={ColorTrack2}
                keys={[
                    { key: 'w', action: 'incGroup:-1' },
                    { key: 'e', action: 'contextToggle:254:1:0' },

                    { key: 'a', action: 'setView:Bass' },
                    { key: 's', action: 'incGroup:+1' },
                    { key: 'd', action: 'noteOn:Bass:60' },
                ]}
                contextValue={0}
            />
            <Common selected={0} hideSequencer track={BassTrack} selectedBackground={ColorTrack2} />
        </View>
    );
}
