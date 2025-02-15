import * as React from '@/libs/react';

import { Keymaps } from '@/libs/nativeComponents/Keymaps';
import { Sequencer } from '@/libs/nativeComponents/Sequencer';
import { View } from '@/libs/nativeComponents/View';
import { VisibilityContext } from '@/libs/nativeComponents/VisibilityContext';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { Common } from '../components/Common';
import {
    btn2,
    btn5,
    btn6,
    btn7,
    btnShift,
    ColorTrack3,
    ColorTrack6,
    KeyInfoPosition,
    PercTrack,
    ScreenWidth
} from '../constants';

export type Props = {
    name: string;
};

export function PercSeq2View({ name }: Props) {
    return (
        <View name={name}>
            <Sequencer bounds={[0, 0, ScreenWidth, 240]} track={PercTrack} />

            <TextGrid
                bounds={KeyInfoPosition}
                rows={[
                    '&icon::toggle::rect &icon::arrowUp::filled ...',
                    'Seq./Perc &icon::arrowDown::filled &icon::musicNote::pixelated',
                    // '!Seq/Bass &icon::arrowDown::filled &icon::musicNote::pixelated',
                ]}
                activeBgColor={ColorTrack3}
            >
                <VisibilityContext index={254} condition="SHOW_WHEN" value={0} />
                <Keymaps
                    keys={[
                        { key: btn2, action: 'incGroup:-1' },
                        { key: btnShift, action: 'contextToggle:254:1:0' },

                        { key: btn5, action: 'setView:Perc' },
                        { key: btn6, action: 'incGroup:+1' },
                        { key: btn7, action: 'noteOn:Perc:60' },
                    ]}
                />
            </TextGrid>
            <Common
                selected={'Perc'}
                hideSequencer
                track={PercTrack}
                selectedBackground={ColorTrack6}
            />
        </View>
    );
}
