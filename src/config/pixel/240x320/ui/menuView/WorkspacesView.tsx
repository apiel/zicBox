import * as React from '@/libs/react';

import { Keymap } from '@/libs/components/Keymap';
import { TextGrid } from '@/libs/components/TextGrid';
import { View } from '@/libs/components/View';
import { VisibilityContext } from '@/libs/components/VisibilityContext';
import { KeyInfoPositionCenter } from '../constants';

export type Props = {
    name: string;
};

export function WorkspacesView({ name }: Props) {
    return (
        <View name={name}>

            <TextGrid
                position={KeyInfoPositionCenter}
                rows={['&empty &icon::arrowUp::filled ...', 'Use &icon::arrowDown::filled Exit']}
            >
                <VisibilityContext index={254} condition="SHOW_WHEN" value={0} />
                <Keymap key="d" action="setView:Home" />
                <Keymap key="e" action="contextToggle:254:1:0" />
            </TextGrid>

            <TextGrid
                position={KeyInfoPositionCenter}
                rows={['New &empty ^...', '&empty &empty &icon::trash']}
            >
                <VisibilityContext index={254} condition="SHOW_WHEN" value={1} />
                <Keymap key="q" action="setView:CreateWorkspace" action2="contextToggle:254:1:0" />
                <Keymap key="e" action="contextToggle:254:1:0" />
            </TextGrid>
        </View>
    );
}
