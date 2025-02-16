import * as React from '@/libs/react';

import { Keyboard } from '@/libs/nativeComponents/Keyboard';
import { Keymaps } from '@/libs/nativeComponents/Keymaps';
import { View } from '@/libs/nativeComponents/View';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { KeyInfoPosition, ScreenWidth } from '../constants';

export type Props = {
    name: string;
};

export function SaveTapeView({ name }: Props) {
    return (
        <View name={name}>
            <Keyboard
                bounds={[0, 40, ScreenWidth, 200]}
                redirectView="Tape"
                audioPlugin="Tape"
                dataId="SAVE"
            >
                <Keymaps
                    keys={[
                        { key: 'q', action: '.type' },
                        { key: 'w', action: '.up' },
                        { key: 'e', action: '.cancel' },
                        { key: 's', action: '.down' },
                        { key: 'a', action: '.left' },
                        { key: 'd', action: '.right' },
                    ]}
                />
            </Keyboard>

            <TextGrid
                bounds={KeyInfoPosition}
                rows={[
                    'Type &icon::arrowUp::filled Cancel',
                    '&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled',
                ]}
            ></TextGrid>
        </View>
    );
}
