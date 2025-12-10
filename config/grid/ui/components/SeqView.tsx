import * as React from '@/libs/react';

import { Sequencer } from '@/libs/nativeComponents/Sequencer';
import { View } from '@/libs/nativeComponents/View';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { rgb } from '@/libs/ui';
import { btn1, btn10, btn2, btn3, btn6, btn7, btn8, btn9, ScreenWidth, SelectorPosition } from '../constants';

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
                    // { key: btn5, action: '.toggleParam', context: { id: 254, value: 0 } },
                    // { key: btn8, action: `noteOn:${synthName}:60`, context: { id: 254, value: 0 } },
                    // { key: btn8, action: `.noteOn`, context: { id: 254, value: 0 } },
                    // { key: btn8, action: '.toggleParam' },
                    // { key: btnUp, action: '.toggleParam', multipleKeyHandler: true },
                    // { key: btnShift, action: '.shift', multipleKeyHandler: true },

                    { key: btn1, action: '.noteOn', context: { id: 254, value: 0 } },
                    { key: btn2, action: '.selectNoteUp', context: { id: 254, value: 0 } },
                    { key: btn3, action: '.toggleParam', context: { id: 254, value: 0 } },

                    { key: btn6, action: '.selectStepLeft', context: { id: 254, value: 0 } },
                    { key: btn7, action: '.selectNoteDown', context: { id: 254, value: 0 } },
                    { key: btn8, action: '.selectStepRight', context: { id: 254, value: 0 } },
                    { key: btn9, action: '.toggleStep', context: { id: 254, value: 0 } },
                    { key: btn10, action: '.deleteStep', context: { id: 254, value: 0 } },
                ]}
                selectedColor={rgb(196, 21, 152)}
                stepColor={color}
                maxStepLen={maxStepLen}
                parameterSelection={parameterSelection}
            />

            {textGrid}
            {/* <Common
                selected={synthName}
                track={track}
                selectedBackground={color}
                synthName={synthName}
            /> */}

            <TextGrid
                bounds={SelectorPosition}
                rows={[
                    `&icon::musicNote::pixelated &icon::arrowUp::filled --- &empty &empty`,
                    `&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled &icon::toggle &icon::trash`,
                ]}
                keys={
                    [
                        // { key: btn4, action: `stop` },
                        // { key: btn5, action: `playPause` },
                    ]
                }
                contextValue={[0]}
            />
        </View>
    );
}
