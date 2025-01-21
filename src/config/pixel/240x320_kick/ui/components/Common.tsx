import * as React from '@/libs/react';

import { SeqProgressBar } from '@/libs/components/SeqProgressBar';
import { rgb } from '@/libs/ui';
import { ScreenWidth } from '../constants';
import { TextGridSel } from './TextGridSel';

export function Common({
    selected,
    hideSequencer,
    track,
    colors,
}: {
    selected: number;
    hideSequencer?: boolean;
    track: number;
    colors?: string[];
}) {
    return (
        <>
            {!hideSequencer && (
                <SeqProgressBar
                    position={[0, 0, ScreenWidth, 5]}
                    seq_plugin="Sequencer"
                    active_color={colors?.[0] || rgb(35, 161, 35)}
                    selection_color={colors?.[0] || rgb(35, 161, 35)}
                    foreground_color={colors?.[1] || rgb(34, 110, 34)}
                    volume_plugin="Volume VOLUME"
                    show_steps
                    track={track}
                />
            )}

            <TextGridSel
                items={['Kick', 'Drums', '^...', 'Fm', 'Menu', '&icon::play::filled']}
                keys={[
                    { key: 'q', action: 'setView:Drum23' },
                    { key: 'w', action: `setView:Snare` },
                    { key: 'e', action: 'contextToggle:254:1:0' },
                    { key: 'a', action: 'setView:Fm' },
                    { key: 's', action: 'setView:Menu' },
                    { key: 'd', action: 'playPause' },
                ]}
                selected={selected}
                contextValue={1}
            />
        </>
    );
}
