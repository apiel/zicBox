import * as React from '@/libs/react';

import { Keymaps } from '@/libs/components/Keymaps';
import { StepEditDrum } from '@/libs/components/StepEditDrum';
import { TextGrid } from '@/libs/components/TextGrid';
import { View } from '@/libs/components/View';
import { VisibilityContext } from '@/libs/components/VisibilityContext';
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
                        position={[0, yy, ScreenWidth, 8]}
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
                position={KeyInfoPosition}
                rows={[
                    '&icon::toggle::rect &icon::arrowUp::filled ...',
                    'Seq./Bass &icon::arrowDown::filled &icon::musicNote::pixelated',
                    // '!Seq/Bass &icon::arrowDown::filled &icon::musicNote::pixelated',
                ]}
                ITEM_BACKGROUND={ColorTrack2}
            >
                <VisibilityContext index={254} condition="SHOW_WHEN" value={0} />
                <Keymaps
                    keys={[
                        { key: 'w', action: 'incGroup:-1' },
                        { key: 'e', action: 'contextToggle:254:1:0' },

                        { key: 'a', action: 'setView:Bass' },
                        { key: 's', action: 'incGroup:+1' },
                        { key: 'd', action: 'noteOn:Bass:60' },
                    ]}
                />
            </TextGrid>
            <Common selected={0} hideSequencer track={BassTrack}  selectedBackground={ColorTrack2} />
        </View>
    );
}
