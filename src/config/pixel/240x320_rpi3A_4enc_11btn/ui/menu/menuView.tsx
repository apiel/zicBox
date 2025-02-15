import * as React from '@/libs/react';

import { Keymap } from '@/libs/nativeComponents/Keymap';
import { List } from '@/libs/nativeComponents/List';
import { TextGrid } from '@/libs/nativeComponents/TextGrid';
import { View } from '@/libs/nativeComponents/View';
import { KeyInfoPosition, ScreenWidth } from '../constants';

export type Props = {
    name: string;
};

export function MenuView({ name }: Props) {
    return (
        <View name={name}>
            <List
                bounds={[0, 0, ScreenWidth, 280]}
                items={['Tape', 'Workspaces', 'Shutdown']}
            >
                <Keymap key="q" action=".setView" />
                <Keymap key="a" action=".setView" />
                <Keymap key="w" action=".up" />
                <Keymap key="s" action=".down" />
            </List>

            <TextGrid
                bounds={KeyInfoPosition}
                rows={['  &icon::arrowUp::filled  ', 'Select &icon::arrowDown::filled Exit']}
            >
                <Keymap key="d" action="setView:Clips" />
            </TextGrid>
        </View>
    );
}
