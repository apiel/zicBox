import * as React from '@/libs/react';

import { Keymap } from '@/libs/nativeComponents/Keymap';
import { List } from '@/libs/nativeComponents/List';
import { View } from '@/libs/nativeComponents/View';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { KeyInfoPositionCenter, ScreenWidth } from '../constants';

export type Props = {
    name: string;
};

export function MenuView({ name }: Props) {
    return (
        <View name={name}>
            <List
                bounds={[0, 0, ScreenWidth, 280]}
                items={['Workspaces', 'Daw', 'More', 'Shutdown']}
            >
                <Keymap key="a" action=".setView" />
                <Keymap key="w" action=".up" />
                <Keymap key="s" action=".down" />
            </List>

            <TextGrid
                bounds={KeyInfoPositionCenter}
                rows={['  &icon::arrowUp::filled  ', 'Select &icon::arrowDown::filled Exit']}
            >
                <Keymap key="d" action="setView:Home" />
            </TextGrid>
        </View>
    );
}
