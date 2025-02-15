import * as React from '@/libs/react';

import { Keymap } from '@/libs/components/Keymap';
import { Text } from '@/libs/components/Text';
import { TextGrid } from '@/libs/components/TextGrid';
import { View } from '@/libs/components/View';
import { KeyInfoPositionCenter, ScreenWidth } from '../constants';

export type Props = {
    name: string;
};

export function ShutdownView({ name }: Props) {
    return (
        <View name={name}>
            <Text
                bounds={[0, 0, ScreenWidth, 280]}
                text="Are you sure to shutdown?"
                color="#ffacac" //#ffacac
                centered
            />

            <TextGrid
                bounds={KeyInfoPositionCenter}
                rows={['&empty &empty &empty', 'Yes &empty No']}
            >
                <Keymap key="q" action="shutdown" />
                <Keymap key="a" action="shutdown" />
                <Keymap key="d" action="setView:Home" />
                <Keymap key="e" action="setView:Home" />
            </TextGrid>
        </View>
    );
}
