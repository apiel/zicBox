import * as React from '@/libs/react';

import { Keyboard } from '@/libs/nativeComponents/Keyboard';
import { Keymaps } from '@/libs/nativeComponents/Keymaps';
import { View } from '@/libs/nativeComponents/View';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { KeyInfoPosition, ScreenWidth } from '../constants';

export type Props = {
    name: string;
};

export function CreateWorkspaceView({ name }: Props) {
    return (
        <View name={name}>
            <Keyboard
                bounds={[0, 40, ScreenWidth, 200]}
                redirectView="Workspaces"
                audioPlugin='SerializeTrack'
                dataId='CREATE_WORKSPACE'
            >
                <Keymaps
                    keys={[
                        { key: 'q', action: '.type' },
                        { key: 'w', action: '.up' },
                        { key: 's', action: '.down' },
                        { key: 'a', action: '.left' },
                        { key: 'd', action: '.right' },
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
