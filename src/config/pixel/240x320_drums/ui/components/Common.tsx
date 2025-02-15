import * as React from '@/libs/react';

import { SeqProgressBar } from '@/libs/components/SeqProgressBar';
import { rgb } from '@/libs/ui';
import { ScreenWidth } from '../constants';
import { TextGridSel } from './TextGridSel';

export function Common({
    selected,
    hideSequencer,
    track,
}: {
    selected: number;
    hideSequencer?: boolean;
    track: number;
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
                items={['Kick', 'Drums', '^...', 'Fm', 'Clips', '&icon::play::filled']}
                keys={[
                    { key: 'q', action: 'setView:Drum23' },
                    { key: 'w', action: `setView:Snare` },
                    { key: 'e', action: 'contextToggle:254:1:0' },
                    { key: 'a', action: 'setView:Fm' },
                    { key: 's', action: 'setView:Clips' },
                    { key: 'd', action: 'playPause' },
                ]}
                selected={selected}
                contextValue={1}
            />
        </>
    );
}
