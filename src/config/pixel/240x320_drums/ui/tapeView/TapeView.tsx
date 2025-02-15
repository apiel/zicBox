import * as React from '@/libs/react';

import { Keymap } from '@/libs/components/Keymap';
import { Keymaps } from '@/libs/components/Keymaps';
import { Tape } from '@/libs/components/Tape';
import { TextGrid } from '@/libs/components/TextGrid';
import { View } from '@/libs/components/View';
import { KeyInfoPosition } from '../constants';

export type Props = {
    name: string;
};

export function TapeView({ name }: Props) {
    return (
        <View name={name}>
            <Tape
                bounds={[0, 0, 240, 280]}
                filename="rec"
                bpm_value="Tempo BPM"
                tape_plugin="Tape PLAY_STOP"
            >
                <Keymap key="q" action=".playStop" />
                <Keymap key="a" action=".left" />
                <Keymap key="d" action=".right" />
            </Tape>

            <TextGrid
                bounds={KeyInfoPosition}
                rows={[
                    '&icon::play::filled Save &empty',
                    '&icon::arrowLeft::filled Exit &icon::arrowRight::filled',
                ]}
            >
                <Keymaps
                    keys={[
                        { key: 'w', action: 'setView:SaveTape' },
                        { key: 's', action: 'setView:Menu' },
                    ]}
                />
            </TextGrid>
        </View>
    );
}
