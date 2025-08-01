import * as React from '@/libs/react';

import { Keyboard } from '@/libs/nativeComponents/Keyboard';
import { View } from '@/libs/nativeComponents/View';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { btn10, btn2, btn4, btn5, btn6, btn7, btn8, btn9, KeyInfoPosition, ScreenWidth } from '../constants';

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
                    { key: btn2, action: '.up' },
                    { key: btn4, action: '.done' },
                    { key: btn5, action: '.cancel' },
                    { key: btn6, action: '.left' },
                    { key: btn7, action: '.down' },
                    { key: btn8, action: '.right' },
                    { key: btn9, action: '.backspace' },
                    { key: btn10, action: '.type' },
                ]}
            />

            <TextGrid
                bounds={KeyInfoPosition}
                rows={[
                    '&empty &icon::arrowUp::filled &empty Done Cancel',
                    '&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled &icon::backspace::filled Type',
                ]}
            ></TextGrid>
        </View>
    );
}
