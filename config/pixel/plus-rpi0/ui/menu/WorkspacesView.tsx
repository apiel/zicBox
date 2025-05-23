import * as React from '@/libs/react';

import { View } from '@/libs/nativeComponents/View';
import { Workspaces } from '@/libs/nativeComponents/Workspaces';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { rgb } from '@/libs/ui';
import { workspaceFolder } from '../../audio';
import {
    btn1,
    btn2,
    btn3,
    btn4,
    btn5,
    btn6,
    btn7,
    btn8,
    KeyInfoPosition,
    ScreenWidth,
} from '../constants';

export type Props = {
    name: string;
};

export function WorkspacesView({ name }: Props) {
    return (
        <View name={name}>
            <Workspaces
                workspaceFolder={workspaceFolder}
                audioPlugin="SerializeTrack"
                bounds={[0, 0, ScreenWidth, 280]}
                keys={[
                    { key: btn5, action: '.data:LOAD_WORKSPACE', context: { id: 254, value: 0 } },
                    { key: btn2, action: '.up', context: { id: 254, value: 0 } },
                    { key: btn6, action: '.down', context: { id: 254, value: 0 } },

                    { key: btn1, action: '.delete', context: { id: 254, value: 1 } },
                    { key: btn5, action: '.delete', context: { id: 254, value: 1 } },
                ]}
            />

            <TextGrid
                bounds={KeyInfoPosition}
                rows={[
                    '&empty &icon::arrowUp::filled &empty &icon::trash',
                    'Use &icon::arrowDown::filled Exit New',
                ]}
                keys={[
                    { key: btn3, action: 'setView:Clips' },
                    { key: btn7, action: 'setView:Clips' },
                    { key: btn4, action: 'contextToggle:254:1:0' },
                    { key: btn8, action: 'setView:CreateWorkspace' },
                ]}
                contextValue={[0]}
            />

            <TextGrid
                bounds={KeyInfoPosition}
                selectedBackground={rgb(173, 99, 99)}
                rows={['!Delete? &empty &empty ^&icon::trash', 'Yes No &empty &empty']}
                contextValue={[1]}
                keys={[
                    { key: btn4, action: 'contextToggle:254:1:0' },
                    { key: btn6, action: 'contextToggle:254:1:0' },
                ]}
            />
        </View>
    );
}
