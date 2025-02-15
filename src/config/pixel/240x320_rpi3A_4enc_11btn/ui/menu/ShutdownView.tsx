import * as React from '@/libs/react';

import { Keymap } from '@/libs/nativeComponents/Keymap';
import { Text } from '@/libs/nativeComponents/Text';
import { TextGrid } from '@/libs/nativeComponents/TextGrid';
import { View } from '@/libs/nativeComponents/View';
import { KeyInfoPosition, ScreenWidth } from '../constants';

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
                bounds={KeyInfoPosition}
                rows={['&empty &empty &empty', 'Yes &empty No']}
            >
                <Keymap key="q" action="shutdown" />
                <Keymap key="a" action="shutdown" />
                <Keymap key="d" action="setView:Clips" />
                <Keymap key="e" action="setView:Clips" />
            </TextGrid>
        </View>
    );
}
