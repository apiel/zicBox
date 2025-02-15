import * as React from '@/libs/react';

import { Keyboard } from '@/libs/nativeComponents/Keyboard';
import { Keymaps } from '@/libs/nativeComponents/Keymaps';
import { TextGrid } from '@/libs/nativeComponents/TextGrid';
import { View } from '@/libs/nativeComponents/View';
import { btn1, btn2, btn5, btn6, btn7, btnShift, KeyInfoPosition, ScreenWidth } from '../constants';

export type Props = {
    name: string;
};

export function SaveTapeView({ name }: Props) {
    return (
        <View name={name}>
            <Keyboard
                bounds={[0, 40, ScreenWidth, 200]}
                redirect_view="Tape"
                done_data="Tape SAVE"
            >
                <Keymaps
                    keys={[
                        { key: btn1, action: '.type' },
                        { key: btn2, action: '.up' },
                        { key: btnShift, action: '.cancel' },
                        { key: btn6, action: '.down' },
                        { key: btn5, action: '.left' },
                        { key: btn7, action: '.right' },
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
