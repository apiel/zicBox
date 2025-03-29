import * as React from '@/libs/react';

import { Sequencer } from '@/libs/nativeComponents/Sequencer';
import { View } from '@/libs/nativeComponents/View';
import { rgb } from '@/libs/ui';
import { btn5, btn8, btnShift, ScreenWidth } from '../constants';
import { Common } from './Common';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
    textGrid: any;
    maxStepLen?: number;
    parameterSelection?: number;
};

export function SeqView({
    name,
    track,
    synthName,
    color,
    textGrid,
    maxStepLen,
    parameterSelection,
}: Props) {
    return (
        <View name={name}>
            <Sequencer
                bounds={[0, 0, ScreenWidth, 270]}
                track={track}
                audioPlugin="Sequencer"
                keys={[
                    { key: btn5, action: '.toggleParam', context: { id: 254, value: 0 } },
                    { key: btn8, action: `noteOn:${synthName}:60`, context: { id: 254, value: 0 } },
                    // { key: btn8, action: '.toggleParam' },
                    // { key: btnUp, action: '.toggleParam', multipleKeyHandler: true },
                    { key: btnShift, action: '.shift', multipleKeyHandler: true },
                ]}
                selectedColor={rgb(196, 21, 152)}
                stepColor={color}
                maxStepLen={maxStepLen}
                parameterSelection={parameterSelection}
            />

            {textGrid}
            <Common
                selected={synthName}
                hideSequencer
                track={track}
                selectedBackground={color}
                synthName={synthName}
            />
        </View>
    );
}
