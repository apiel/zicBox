import * as React from '@/libs/react';

import { List } from '@/libs/nativeComponents/List';
import { Text } from './libs/nativeComponents/Text';

export type Props = {
    name: string;
};

export function Yo2() {
    return (
        <>
            <Text fontSize={16} text="yo2" bounds={[0, 0, 100, 16]} centered />
            <List
                bounds={[0, 0, 123, 280]}
                items={['yoyoyoy']}
                keys={[{ key: 0, action: '.setView' }]}
            />
        </>
    );
}
