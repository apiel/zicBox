import * as React from '@/libs/react';

import { Keyboard } from '@/libs/nativeComponents/Keyboard';
import { Keymaps } from '@/libs/nativeComponents/Keymaps';
import { View } from '@/libs/nativeComponents/View';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { btn1, btn2, btn5, btn6, btn7, KeyInfoPosition, ScreenWidth } from '../constants';

export type Props = {
    name: string;
};

export function CreateWorkspaceView({ name }: Props) {
    return (
        <View name={name}>
            <Keyboard
                bounds={[0, 40, ScreenWidth, 200]}
                redirectView="Workspaces"
                audioPlugin="SerializeTrack"
                dataId="CREATE_WORKSPACE"
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
                bounds={KeyInfoPosition}
                rows={[
                    'Type &icon::arrowUp::filled &empty',
                    '&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled',
                ]}
            ></TextGrid>
        </View>
    );
}
