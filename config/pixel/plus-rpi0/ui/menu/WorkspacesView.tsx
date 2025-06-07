import * as React from '@/libs/react';

import { View } from '@/libs/nativeComponents/View';
import { Workspaces } from '@/libs/nativeComponents/Workspaces';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { rgb } from '@/libs/ui';
import { workspaceFolder } from '../../audio';
import { MainKeys } from '../components/Common';
import {
    btn10,
    btn2,
    btn6,
    btn7,
    btn8,
    btn9,
    ScreenWidth,
    SelectorPosition
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
                    { key: btn6, action: '.data:LOAD_WORKSPACE', context: { id: 254, value: 0 } },
                    { key: btn2, action: '.up', context: { id: 254, value: 0 } },
                    { key: btn7, action: '.down', context: { id: 254, value: 0 } },

                    { key: btn10, action: '.delete', context: { id: 254, value: 1 } },
                ]}
            />

            <TextGrid
                bounds={SelectorPosition}
                rows={[
                    '&empty &icon::arrowUp::filled &empty &empty &empty',
                    'Use &icon::arrowDown::filled Exit New &icon::trash',
                ]}
                keys={[
                    { key: btn8, action: 'setView:Clips' },
                    { key: btn9, action: 'setView:CreateWorkspace' },
                    { key: btn10, action: 'contextToggle:254:1:0' },
                ]}
                contextValue={[0]}
            />

            <TextGrid
                bounds={SelectorPosition}
                selected="Delete?"
                selectedBackground={rgb(173, 99, 99)}
                rows={['Delete? &empty &empty &empty &empty', 'Yes No &empty &empty ^&icon::trash']}
                contextValue={[1]}
                keys={[
                    { key: btn6, action: 'contextToggle:254:1:0' },
                    { key: btn7, action: 'contextToggle:254:1:0' },
                ]}
            />

            <MainKeys synthName="Drum1" forcePatchView />
        </View>
    );
}
