import * as React from '@/libs/react';

import { SeqProgressBar } from '@/libs/components/SeqProgressBar';
import { rgb } from '@/libs/ui';
import { btn1, btn2, btn5, btn6, btn7, btnShift, ScreenWidth } from '../constants';
import { TextGridSel } from './TextGridSel';

export function Common({
    selected,
    hideSequencer,
    track,
    selectedBackground,
}: {
    selected: number;
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
                items={['Kick', 'Sample', '^...', 'Bass', 'Clips', '&icon::play::filled']}
                keys={[
                    { key: btn1, action: 'setView:Drum23' },
                    { key: btn2, action: `setView:Sample` },
                    { key: btnShift, action: 'contextToggle:254:1:0' },

                    { key: btn5, action: 'setView:Bass' },
                    { key: btn6, action: `setView:Clips` },
                    { key: btn7, action: 'playPause' },
                ]}
                selected={selected}
                contextValue={1}
                {...(selectedBackground && { ITEM_BACKGROUND: selectedBackground })}
            />
        </>
    );
}
