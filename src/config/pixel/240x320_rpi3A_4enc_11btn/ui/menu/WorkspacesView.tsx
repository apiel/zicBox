import * as React from '@/libs/react';

import { Keymap } from '@/libs/components/Keymap';
import { Keymaps } from '@/libs/components/Keymaps';
import { TextGrid } from '@/libs/components/TextGrid';
import { View } from '@/libs/components/View';
import { VisibilityContext } from '@/libs/components/VisibilityContext';
import { Workspaces } from '@/libs/components/Workspaces';
import { btn2, btn5, btn6, btn7, KeyInfoPosition, ScreenWidth } from '../constants';

export type Props = {
    name: string;
};

export function WorkspacesView({ name }: Props) {
    return (
        <View name={name}>
            <Workspaces plugin="SerializeTrack" position={[0, 0, ScreenWidth, 280]}>
                <Keymaps
                    keys={[
                        { key: btn5, action: '.data:LOAD_WORKSPACE', context: '254:0' },
                        { key: btn2, action: '.up', context: '254:0' },
                        { key: btn6, action: '.down', context: '254:0' },
                        { key: btn7, action: '.delete', context: '254:1' },
                    ]}
                />
            </Workspaces>

            <TextGrid
                position={KeyInfoPosition}
                rows={['&empty &icon::arrowUp::filled ...', 'Use &icon::arrowDown::filled Exit']}
            >
                <VisibilityContext index={254} condition="SHOW_WHEN" value={0} />
                <Keymap key="d" action="setView:Clips" />
                <Keymap key="e" action="contextToggle:254:1:0" />
            </TextGrid>

            <TextGrid
                position={KeyInfoPosition}
                rows={['New &empty ^...', '&empty &empty &icon::trash']}
            >
                <VisibilityContext index={254} condition="SHOW_WHEN" value={1} />
                <Keymap key="q" action="setView:CreateWorkspace" action2="contextToggle:254:1:0" />
                <Keymap key="e" action="contextToggle:254:1:0" />
            </TextGrid>
        </View>
    );
}
