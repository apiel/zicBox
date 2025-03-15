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
export const Synth1 = 'Synth1';
export const Synth2 = 'Synth2';
export const Synth3 = 'Synth3';
export const Clips = 'Clips';
export const Perc = 'Perc';


export function Common({
    hideSequencer,
    track,
    selectedBackground,
    selected,
}: {
    hideSequencer?: boolean;
    track: number;
    selectedBackground?: string;
    selected: string;
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
                rows={[`${Kick} ${Synth1} ${Synth2} ${Synth3}`, `${Bass} ${Perc} Nix ${Clips}`]}
                keys={[
                    { key: btn1, action: 'setView:Kick' },
                    { key: btn2, action: 'setView:Synth1' },
                    { key: btn3, action: 'setView:Synth2' },
                    { key: btn4, action: 'setView:Synth3' },

                    { key: btn5, action: `setView:Bass` },
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
