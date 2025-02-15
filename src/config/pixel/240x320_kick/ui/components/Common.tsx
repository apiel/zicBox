import * as React from '@/libs/react';

import { SeqProgressBar } from '@/libs/nativeComponents/SeqProgressBar';
import { rgb } from '@/libs/ui';
import { ScreenWidth } from '../constants';
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
                    bounds={[0, 0, ScreenWidth, 5]}
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
                    { key: 'q', action: 'setView:Drum23' },
                    { key: 'w', action: `setView:Sample` },
                    { key: 'e', action: 'contextToggle:254:1:0' },

                    { key: 'a', action: 'setView:Bass' },
                    { key: 's', action: `setView:Clips` },
                    { key: 'd', action: 'playPause' },
                ]}
                selected={selected}
                contextValue={1}
                {...(selectedBackground && { ITEM_BACKGROUND: selectedBackground })}
            />
        </>
    );
}
