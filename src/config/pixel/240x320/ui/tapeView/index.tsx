import * as React from '@/libs/react';

import { Keymap } from '@/libs/components/Keymap';
import { Tape } from '@/libs/components/Tape';
import { View } from '@/libs/components/View';
import { ValueBpm } from '../components/ValueBpm';
import { TextGridTape } from './TextGrid';

export type Props = {
    name: string;
};

export function TapeView({ name }: Props) {
    return (
        <View name={name}>
            <Tape bounds={[0, 0, 240, 280]} filename="rec" bpm_value="Tempo BPM" tape_plugin="Tape PLAY_STOP">
                <Keymap key="q" action=".playStop" />
                <Keymap key="a" action=".left" />
                <Keymap key="d" action=".right" />
            </Tape>

            <ValueBpm />

            <TextGridTape />
        </View>
    );
}
