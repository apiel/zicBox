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
    btn8,
    btnDown,
    btnShift,
    btnUp,
    KeyInfoPosition,
    ScreenWidth,
} from '../constants';
import { SideInfo } from './SideInfo';

export const Kick = 'Kick';
export const Bass = 'Bass';
export const Fm1 = 'Fm1';
export const Synth = 'Synth';
export const Sample1 = 'Sampl1';
export const Sample2 = 'Sampl2';
export const Sample3 = 'Sampl3';
export const Clips = 'Clips';
export const Perc = 'Perc';

type ItemType =
    | typeof Kick
    | typeof Bass
    | typeof Fm1
    | typeof Synth
    | typeof Sample1
    | typeof Sample2
    | typeof Sample3
    | typeof Clips
    | typeof Perc;

export function Common({
    selected,
    hideSequencer,
    track,
    selectedBackground,
}: {
    selected: ItemType;
    hideSequencer?: boolean;
    track: number;
    selectedBackground?: string;
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

            <TextGrid
                bounds={KeyInfoPosition}
                rows={[`${Kick} ${Bass} ${Fm1} ${Synth}`, `${Sample1} ${Perc} ${Sample3} ${Clips}`]}
                keys={[
                    { key: btn1, action: 'setView:Drum23' },
                    { key: btn2, action: 'setView:Bass' },
                    { key: btn3, action: 'setView:Fm1' },
                    { key: btn4, action: 'setView:Synth' },

                    { key: btn5, action: `setView:Sample` },
                    { key: btn6, action: `setView:Perc` },
                    { key: btn8, action: `setView:Clips` },

                    { key: btnUp, action: 'setView:Menu' },
                    { key: btnDown, action: 'playPause' },
                    { key: btnShift, action: 'contextToggle:254:1:0' },
                ]}
                selected={selected}
                contextValue={1}
                {...(selectedBackground && { selectedBackground })}
            />
            <SideInfo up="*" down="&icon::play::filled" ctxValue={1} />
        </>
    );
}
