import * as React from '@/libs/react';

import { SeqProgressBar } from '@/libs/components/SeqProgressBar';
import { rgb } from '@/libs/ui';
import { btn1, btn2, btn5, btn8, btnDown, btnShift, ScreenWidth } from '../constants';
import { TextGridSel } from './TextGridSel';

export const Kick = 'Kick';
export const Bass = 'Bass';
export const Fm1 = 'Fm1';
export const Fm2 = 'Fm2';
export const Sample1 = 'Sampl1';
export const Sample2 = 'Sampl2';
export const Sample3 = 'Sampl3';
export const Clips = 'Clips';

type ItemType =
    | typeof Kick
    | typeof Bass
    | typeof Fm1
    | typeof Fm2
    | typeof Sample1
    | typeof Sample2
    | typeof Sample3
    | typeof Clips;

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
                    position={[0, 0, ScreenWidth, 5]}
                    seq_plugin="Sequencer"
                    active_color={rgb(35, 161, 35)}
                    selection_color={rgb(35, 161, 35)}
                    foreground_color={rgb(34, 110, 34)}
                    volume_plugin="TrackFx VOLUME"
                    show_steps
                    track={track}
                />
            )}

            <TextGridSel
                rows={[
                    `${Kick} ${Bass} ${Fm1} ${Fm2}`,
                    `${Sample1} ${Sample2} ${Sample3} ${Clips}`,
                ]}
                keys={[
                    { key: btn1, action: 'setView:Drum23' },
                    { key: btn2, action: 'setView:Bass' },

                    { key: btn5, action: `setView:Sample` },
                    { key: btn8, action: `setView:Clips` },

                    { key: btnShift, action: 'contextToggle:254:1:0' },
                    { key: btnDown, action: 'playPause' },
                ]}
                selected={selected}
                contextValue={1}
                {...(selectedBackground && { selectedBackground })}
            />
        </>
    );
}
