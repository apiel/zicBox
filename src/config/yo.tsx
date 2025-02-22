import * as React from '@/libs/react';

import { List } from '@/libs/nativeComponents/List';
import { View } from '@/libs/nativeComponents/View';

export type Props = {
    name: string;
};

export function Yo({ name }: Props) {
    return (
        <View name={name}>
            <List
                bounds={[0, 0, 123, 280]}
                items={['Tape', 'Workspaces', 'Shutdown']}
                keys={[
                    { key: 0, action: '.setView' },
                ]}
            />
        </View>
    );
}
