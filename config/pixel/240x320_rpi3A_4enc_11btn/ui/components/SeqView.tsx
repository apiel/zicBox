import * as React from '@/libs/react';

import { Sequencer } from '@/libs/nativeComponents/Sequencer';
import { View } from '@/libs/nativeComponents/View';
import { rgb } from '@/libs/ui';
import {
    btnShift,
    btnUp,
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
                stepColor={color}
                maxStepLen={1}
            />

            {textGrid}
            <Common
                selected={synthName}
                hideSequencer
                track={track}
                selectedBackground={color}
            />
        </View>
    );
}
