import * as React from '@/libs/react';

import { Keyboard } from '@/libs/components/Keyboard';
import { Keymaps } from '@/libs/components/Keymaps';
import { TextGrid } from '@/libs/components/TextGrid';
import { View } from '@/libs/components/View';
import { btn1, btn2, btn5, btn6, btn7, KeyInfoPosition, ScreenWidth } from '../constants';

export type Props = {
    name: string;
};

export function CreateWorkspaceView({ name }: Props) {
    return (
        <View name={name}>
            <Keyboard
                position={[0, 40, ScreenWidth, 200]}
                redirect_view="Workspaces"
                done_data="SerializeTrack CREATE_WORKSPACE"
            >
                <Keymaps
                    keys={[
                        { key: btn1, action: '.type' },
                        { key: btn2, action: '.up' },
                        { key: btn6, action: '.down' },
                        { key: btn5, action: '.left' },
                        { key: btn7, action: '.right' },
                    ]}
                />
            </Keyboard>

            <TextGrid
                position={KeyInfoPosition}
                rows={[
                    'Type &icon::arrowUp::filled &empty',
                    '&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled',
                ]}
            ></TextGrid>
        </View>
    );
}
