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
    ColorTrack1,
    ColorTrack3,
    Drum23Track,
    KeyInfoPosition,
    ScreenWidth,
} from '../constants';

export type Props = {
    name: string;
};

export function Drum23SeqView({ name }: Props) {
    return (
        <View name={name}>
            <Sequencer
                bounds={[0, 0, ScreenWidth, 270]}
                track={Drum23Track}
                audioPlugin="Sequencer"
                keys={[
                    { key: btnUp, action: '.toggleParam' },
                    { key: btnShift, action: '.shift' },
                ]}
                selectedColor={rgb(196, 21, 152)}
                stepColor={ColorTrack1}
                maxStepLen={1}
            />

            <TextGrid
                bounds={KeyInfoPosition}
                rows={[
                    '&icon::toggle::rect &icon::arrowUp::filled ...',
                    'Seq./Kick &icon::arrowDown::filled &icon::musicNote::pixelated',
                    // '!Seq/Bass &icon::arrowDown::filled &icon::musicNote::pixelated',
                ]}
                selectedBackground={ColorTrack3}
                keys={[
                    { key: btn2, action: 'incGroup:-1' },
                    { key: btnShift, action: 'contextToggle:254:1:0' },

                    { key: btn5, action: 'setView:Drum23' },
                    { key: btn6, action: 'incGroup:+1' },
                    { key: btn7, action: 'noteOn:Drum23:60' },
                ]}
                contextValue={0}
            />
            <Common
                selected={'Kick'}
                hideSequencer
                track={Drum23Track}
                selectedBackground={ColorTrack1}
            />
        </View>
    );
}
