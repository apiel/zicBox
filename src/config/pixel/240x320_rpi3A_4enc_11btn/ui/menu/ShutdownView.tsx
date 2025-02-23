import * as React from '@/libs/react';

import { Text } from '@/libs/nativeComponents/Text';
import { View } from '@/libs/nativeComponents/View';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
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
                keys={[
                    { key: 'q', action: 'shutdown' },
                    { key: 'a', action: 'shutdown' },
                    { key: 'd', action: 'setView:Clips' },
                    { key: 'e', action: 'setView:Clips' },
                ]}
            />

            <TextGrid
                bounds={KeyInfoPosition}
                rows={['&empty &empty &empty', 'Yes &empty No']}
            />
        </View>
    );
}
