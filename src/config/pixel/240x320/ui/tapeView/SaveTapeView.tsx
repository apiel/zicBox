import * as React from '@/libs/react';

import { Keyboard } from '@/libs/components/Keyboard';
import { Keymaps } from '@/libs/components/Keymaps';
import { TextGrid } from '@/libs/components/TextGrid';
import { View } from '@/libs/components/View';
import { KeyInfoPositionCenter, ScreenWidth } from '../constants';

export type Props = {
    name: string;
};

export function SaveTapeView({ name }: Props) {
    return (
        <View name={name}>
            <Keyboard
                position={[0, 40, ScreenWidth, 200]}
                redirect_view="Tape"
                done_data="Tape SAVE"
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
                position={KeyInfoPositionCenter}
                rows={[
                    'Type &icon::arrowUp::filled Cancel',
                    '&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled',
                ]}
            ></TextGrid>
        </View>
    );
}
