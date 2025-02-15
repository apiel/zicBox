import * as React from '@/libs/react';

import { Keyboard } from '@/libs/components/Keyboard';
import { Keymaps } from '@/libs/components/Keymaps';
import { TextGrid } from '@/libs/components/TextGrid';
import { View } from '@/libs/components/View';
import { KeyInfoPosition, ScreenWidth } from '../constants';

export type Props = {
    name: string;
};

export function CreateWorkspaceView({ name }: Props) {
    return (
        <View name={name}>
            <Keyboard
                bounds={[0, 40, ScreenWidth, 200]}
                redirect_view="Workspaces"
                done_data="SerializeTrack CREATE_WORKSPACE"
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
