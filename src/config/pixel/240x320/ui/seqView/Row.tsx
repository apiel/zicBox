import * as React from '@/libs/react';

import { Keymaps } from '@/libs/components/Keymaps';
import { SeqSynthBar } from '@/libs/components/SeqSynthBar';
import { ScreenWidth } from '../constants';

export type Props = {
    y: number;
    group: number;
    track: number;
    name_color: string;
    select_menu_context: string;
};

export const RowH = 14;

function RowKeymaps({ synth }: { synth: string }) {
    return (
        <Keymaps
            keys={[
                { key: 'q', action: `.toggleOrPlayNote:${synth}`, context: '254:0' },
                { key: 'a', action: '.left', context: '254:0' },
                { key: 'd', action: '.right', context: '254:0' },
                { key: 'q', action: '.toggle', context: '254:1' },
            ]}
        />
    );
}

export function rowDrum23({ y, group, track, name_color, select_menu_context }: Props) {
    return (
        <SeqSynthBar
            position={[0, y, ScreenWidth, RowH]}
            seq_plugin="Sequencer"
            track={track}
            group={group}
            name_color={name_color}
            name={track}
            items="Click Freq. Amp. Wave FX"
            volume_plugin="Volume VOLUME"
            select_menu_context={select_menu_context}
        >
            <RowKeymaps synth={'Drum23'} />
        </SeqSynthBar>
    );
}

export function rowDrumSample({ y, group, track, name_color, select_menu_context }: Props) {
    return (
        <SeqSynthBar
            position={[0, y, ScreenWidth, RowH]}
            seq_plugin="Sequencer"
            track={track}
            group={group}
            name_color={name_color}
            name={track}
            items="Sample"
            volume_plugin="Volume VOLUME"
            select_menu_context={select_menu_context}
        >
            <RowKeymaps synth={'DrumSample'} />
        </SeqSynthBar>
    );
}
