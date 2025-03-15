import * as React from '@/libs/react';

import { Sequencer } from '@/libs/nativeComponents/Sequencer';
import { View } from '@/libs/nativeComponents/View';
import { rgb } from '@/libs/ui';
import {
    btnShift,
    btnUp,
    ColorTrack1,
    ScreenWidth
} from '../constants';
import { Common } from './Common';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
    textGrid: any;
};

export function SeqView({ name, track, synthName, color, textGrid }: Props) {
    return (
        <View name={name}>
            <Sequencer
                bounds={[0, 0, ScreenWidth, 270]}
                track={track}
                audioPlugin="Sequencer"
                keys={[
                    { key: btnUp, action: '.toggleParam' },
                    { key: btnShift, action: '.shift' },
                ]}
                selectedColor={rgb(196, 21, 152)}
                stepColor={ColorTrack1}
                maxStepLen={1}
            />

            {/* <TextGrid
                bounds={KeyInfoPosition}
                rows={[
                    '&icon::toggle::rect &icon::arrowUp::filled ...',
                    'Seq./Kick &icon::arrowDown::filled &icon::musicNote::pixelated',
                    // '!Seq/Bass &icon::arrowDown::filled &icon::musicNote::pixelated',
                ]}
                selectedBackground={color}
                keys={[
                    { key: btn2, action: 'incGroup:-1' },
                    { key: btnShift, action: 'contextToggle:254:1:0' },

                    { key: btn5, action: `setView:${synthName}` },
                    { key: btn6, action: 'incGroup:+1' },
                    { key: btn7, action: `noteOn:${synthName}:60` },
                ]}
                contextValue={0}
            /> */}
            {textGrid}
            <Common
                selected={name}
                hideSequencer
                track={track}
                selectedBackground={color}
            />
        </View>
    );
}
