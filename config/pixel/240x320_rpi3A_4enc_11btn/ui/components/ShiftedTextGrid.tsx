import * as React from '@/libs/react';

import { TextGrid } from '@/libs/tsComponents/TextGrid';
import {
    btn1,
    btn2,
    btn3,
    btn4,
    btn5,
    btn6,
    btn7,
    btn8,
    btnDown,
    btnShift,
    btnUp,
    KeyInfoPosition
} from '../constants';

export const Kick = 'Kick';
export const Synth1 = 'Synth1';
export const Synth2 = 'Synth2';
export const Synth3 = 'Synth3';
export const Clips = 'Clips';
export const Perc = 'Perc';
export const Sample1 = 'Sample1';
export const Sample2 = 'Sample2';
export const Sample3 = 'Sample3';

export function ShiftedTextGrid({
    selectedBackground,
    selected,
}: {
    selectedBackground?: string;
    selected: string;
}) {
    return (
        <>
            <TextGrid
                bounds={KeyInfoPosition}
                rows={[
                    `${Kick} ${Synth1} ${Synth2} ${Synth3}`,
                    `${Perc} ${Sample1} ${Sample2} ${Sample3}`,
                ]}
                keys={[
                    { key: btn1, action: 'setView:Kick' },
                    { key: btn2, action: 'setView:Synth1' },
                    { key: btn3, action: 'setView:Synth2' },
                    { key: btn4, action: 'setView:Synth3' },

                    { key: btn5, action: `setView:Perc` },
                    { key: btn6, action: `setView:Sample1` },
                    { key: btn7, action: `setView:Sample2` },
                    { key: btn8, action: `setView:Sample3` },

                    { key: btnUp, action: 'setView:Clips' },
                    { key: btnDown, action: 'playPause' },
                    { key: btnShift, action: 'contextToggle:254:1:0' },
                ]}
                selected={selected}
                contextValue={[1]}
                {...(selectedBackground && { selectedBackground })}
            />

            <TextGrid
                bounds={KeyInfoPosition}
                rows={[
                    `Menu &empty &empty &empty`,
                    `&icon::play::filled &empty &empty &empty`,
                ]}
                keys={[
                    { key: btn1, action: 'setView:Menu' },
                    // { key: btn2, action: 'setView:Synth1' },
                    // { key: btn3, action: 'setView:Synth2' },
                    // { key: btn4, action: 'setView:Synth3' },

                    { key: btn5, action: `playPause` },
                    // { key: btn6, action: `setView:Sample1` },
                    // { key: btn7, action: `setView:Sample2` },
                    // { key: btn8, action: `setView:Sample3` },

                    { key: btnUp, action: 'contextToggle:253:1:0' },
                    // { key: btnDown, action: 'playPause' },
                    // { key: btnShift, action: 'contextToggle:254:1:0' },
                ]}
                selected={selected}
                contextValue={[0, 1]}
                {...(selectedBackground && { selectedBackground })}
            />
        </>
    );
}
