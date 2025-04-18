import * as React from '@/libs/react';

import { View } from '@/libs/nativeComponents/View';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { btn2, btn6, KeyInfoPosition } from '../constants';

export type Props = {
    name: string;
};

export function TapeView({ name }: Props) {
    return (
        <View name={name}>
            {/* <Tape
                bounds={[0, 0, 240, 280]}
                filename="rec"
                bpm_audioPlugin="Tempo" param="BPM"
                tape_plugin="Tape PLAY_STOP"
            >
                <Keymap key="q" action=".playStop" />
                <Keymap key="a" action=".left" />
                <Keymap key="d" action=".right" />
            </Tape> */}

            <TextGrid
                bounds={KeyInfoPosition}
                rows={[
                    '&icon::play::filled Save &empty',
                    '&icon::arrowLeft::filled Exit &icon::arrowRight::filled',
                ]}
                keys={[
                    { key: btn2, action: 'setView:SaveTape' },
                    { key: btn6, action: 'setView:Menu' },
                ]}
            />
        </View>
    );
}
