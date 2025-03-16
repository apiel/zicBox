import * as React from '@/libs/react';

import { Sequencer } from '@/libs/nativeComponents/Sequencer';
import { View } from '@/libs/nativeComponents/View';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { rgb } from '@/libs/ui';
import { Common } from '../components/Common';
import {
    btn2,
    btn5,
    btn6,
    btn7,
    btnShift,
    btnUp,
    ColorTrack5,
    KeyInfoPosition,
    PercTrack,
    ScreenWidth
} from '../constants';

export type Props = {
    name: string;
};

export function PercSeqView({ name }: Props) {
    return (
        <View name={name}>
            <Sequencer
                bounds={[0, 0, ScreenWidth, 270]}
                track={PercTrack}
                audioPlugin="Sequencer"
                keys={[{ key: btnUp, action: '.toggleParam' }]}
                selectedColor={rgb(230, 220, 253)}
                stepColor={ColorTrack5}
            />

            <TextGrid
                bounds={KeyInfoPosition}
                rows={[
                    '&icon::toggle::rect &icon::arrowUp::filled ...',
                    'Seq./Perc &icon::arrowDown::filled &icon::musicNote::pixelated',
                    // '!Seq/Bass &icon::arrowDown::filled &icon::musicNote::pixelated',
                ]}
                selectedBackground={ColorTrack5}
                keys={[
                    { key: btn2, action: 'incGroup:-1' },
                    { key: btnShift, action: 'contextToggle:254:1:0' },

                    { key: btn5, action: 'setView:Perc' },
                    { key: btn6, action: 'incGroup:+1' },
                    { key: btn7, action: 'noteOn:Perc:60' },
                ]}
                contextValue={[0]}
            />
            <Common
                selected={'Perc'}
                hideSequencer
                track={PercTrack}
                selectedBackground={ColorTrack5}
                synthName="Perc"
            />
        </View>
    );
}
