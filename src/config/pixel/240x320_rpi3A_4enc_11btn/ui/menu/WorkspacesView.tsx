import * as React from '@/libs/react';

import { Keymap } from '@/libs/nativeComponents/Keymap';
import { View } from '@/libs/nativeComponents/View';
import { VisibilityContext } from '@/libs/nativeComponents/VisibilityContext';
import { Workspaces } from '@/libs/nativeComponents/Workspaces';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { btn2, btn5, btn6, btn7, KeyInfoPosition, ScreenWidth } from '../constants';

export type Props = {
    name: string;
};

export function WorkspacesView({ name }: Props) {
    return (
        <View name={name}>
            <Workspaces
                audioPlugin="SerializeTrack"
                bounds={[0, 0, ScreenWidth, 280]}
                keys={[
                    { key: btn5, action: '.data:LOAD_WORKSPACE', context: { id: 254, value: 0 } },
                    { key: btn2, action: '.up', context: { id: 254, value: 0 } },
                    { key: btn6, action: '.down', context: { id: 254, value: 0 } },
                    { key: btn7, action: '.delete', context: { id: 254, value: 1 } },
                ]}
            />

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
