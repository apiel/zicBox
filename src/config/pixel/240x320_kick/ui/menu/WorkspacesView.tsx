import * as React from '@/libs/react';

import { Keymap } from '@/libs/nativeComponents/Keymap';
import { Keymaps } from '@/libs/nativeComponents/Keymaps';
import { TextGrid } from '@/libs/nativeComponents/TextGrid';
import { View } from '@/libs/nativeComponents/View';
import { VisibilityContext } from '@/libs/nativeComponents/VisibilityContext';
import { Workspaces } from '@/libs/nativeComponents/Workspaces';
import { KeyInfoPosition, ScreenWidth } from '../constants';

export type Props = {
    name: string;
};

export function WorkspacesView({ name }: Props) {
    return (
        <View name={name}>
            <Workspaces plugin="SerializeTrack" bounds={[0, 0, ScreenWidth, 280]}>
                <Keymaps
                    keys={[
                        { key: 'a', action: '.data:LOAD_WORKSPACE', context: '254:0' },
                        { key: 'w', action: '.up', context: '254:0' },
                        { key: 's', action: '.down', context: '254:0' },
                        { key: 'd', action: '.delete', context: '254:1' },
                    ]}
                />
            </Workspaces>

            <TextGrid
                bounds={KeyInfoPosition}
                rows={['&empty &icon::arrowUp::filled ...', 'Use &icon::arrowDown::filled Exit']}
            >
                <VisibilityContext index={254} condition="SHOW_WHEN" value={0} />
                <Keymap key="d" action="setView:Clips" />
                <Keymap key="e" action="contextToggle:254:1:0" />
            </TextGrid>

            <TextGrid
                bounds={KeyInfoPosition}
                rows={['New &empty ^...', '&empty &empty &icon::trash']}
            >
                <VisibilityContext index={254} condition="SHOW_WHEN" value={1} />
                <Keymap key="q" action="setView:CreateWorkspace" action2="contextToggle:254:1:0" />
                <Keymap key="e" action="contextToggle:254:1:0" />
            </TextGrid>
        </View>
    );
}
