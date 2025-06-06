import * as React from '@/libs/react';

import { Keyboard } from '@/libs/nativeComponents/Keyboard';
import { View } from '@/libs/nativeComponents/View';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { btn1, btn2, btn3, btn4, btn5, btn6, btn7, btn8, KeyInfoPosition, ScreenWidth } from '../constants';

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
                keys={[
                    { key: btn1, action: '.type' },
                    { key: btn2, action: '.up' },
                    { key: btn3, action: '.done' },
                    { key: btn4, action: '.cancel' },
                    { key: btn5, action: '.left' },
                    { key: btn6, action: '.down' },
                    { key: btn7, action: '.right' },
                    { key: btn8, action: '.backspace' },
                ]}
            />

            <TextGrid
                bounds={KeyInfoPosition}
                rows={[
                    'Type &icon::arrowUp::filled Done Cancel',
                    '&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled &icon::backspace::filled',
                ]}
            ></TextGrid>
        </View>
    );
}
