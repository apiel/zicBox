import * as React from '@/libs/react';

import { SeqProgressBar } from '@/libs/nativeComponents/SeqProgressBar';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { rgb } from '@/libs/ui';
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
    KeyInfoPosition,
    ScreenWidth,
} from '../constants';
import { ShiftedTextGrid } from './ShiftedTextGrid';

export const Kick = 'Kick';
export const Synth1 = 'Synth1';
export const Synth2 = 'Synth2';
export const Synth3 = 'Synth3';
export const Clips = 'Clips';
export const Perc = 'Perc';
export const Sample1 = 'Sample1';
export const Sample2 = 'Sample2';
export const Sample3 = 'Sample3';

export function Common({
    hideSequencer,
    track,
    selectedBackground,
    selected,
    synthName,
}: {
    hideSequencer?: boolean;
    track: number;
    selectedBackground?: string;
    selected: string;
    synthName: string;
}) {
    return (
        <>
            {!hideSequencer && (
                <SeqProgressBar
                    bounds={[0, 0, ScreenWidth, 5]}
                    audioPlugin="Sequencer"
                    activeColor={rgb(35, 161, 35)}
                    selectionColor={rgb(35, 161, 35)}
                    fgColor={rgb(34, 110, 34)}
                    volumePlugin={{ plugin: 'TrackFx', param: 'VOLUME' }}
                    showSteps
                    track={track}
                />
            )}

            <ShiftedTextGrid selectedBackground={selectedBackground} selected={selected} />

            <TextGrid
                bounds={KeyInfoPosition}
                // C major pentatonic scale
                // rows={['A4 C5 D5 E5', 'C4 D4 E4 G4']}
                // keys={[
                //     { key: btn1, action: `noteOn:${synthName}:69` },
                //     { key: btn2, action: `noteOn:${synthName}:72` },
                //     { key: btn3, action: `noteOn:${synthName}:74` },
                //     { key: btn4, action: `noteOn:${synthName}:76` },

                //     { key: btn5, action: `noteOn:${synthName}:60` },
                //     { key: btn6, action: `noteOn:${synthName}:62` },
                //     { key: btn7, action: `noteOn:${synthName}:64` },
                //     { key: btn8, action: `noteOn:${synthName}:67` },

                //     { key: btnUp, action: `noteOn:${synthName}:60` },
                //     { key: btnDown, action: 'contextToggle:253:1:0' },
                //     // { key: btnDown, action: 'debug:yoyoyoyo' },
                //     { key: btnShift, action: 'contextToggle:254:1:0' },
                // ]}

                // C Major scale
                rows={['G4 A4 B4 C5', 'C4 D4 E4 F4']}
                keys={[
                    { key: btn1, action: `noteOn:${synthName}:67` },
                    { key: btn2, action: `noteOn:${synthName}:69` },
                    { key: btn3, action: `noteOn:${synthName}:71` },
                    { key: btn4, action: `noteOn:${synthName}:72` },

                    { key: btn5, action: `noteOn:${synthName}:60` },
                    { key: btn6, action: `noteOn:${synthName}:62` },
                    { key: btn7, action: `noteOn:${synthName}:64` },
                    { key: btn8, action: `noteOn:${synthName}:65` },

                    { key: btnUp, action: 'contextToggle:253:1:0' },
                    { key: btnDown, action: 'contextToggle:252:1:0' },
                    // { key: btnDown, action: 'debug:yoyoyoyo' },
                    { key: btnShift, action: 'contextToggle:254:1:0' },
                ]}
                selected={selected}
                contextValue={[0, 0, 1]}
            />
        </>
    );
}
