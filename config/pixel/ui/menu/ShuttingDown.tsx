import * as React from '@/libs/react';

import { Pixels } from '@/libs/nativeComponents/Pixels';
import { Text } from '@/libs/nativeComponents/Text';
import { View } from '@/libs/nativeComponents/View';
import { ScreenHeight, ScreenWidth } from '../constants';

export type Props = {
    name: string;
};

export function ShuttingDown({ name }: Props) {
    return (
        <View name={name}>
            <Pixels bounds={[0, 0, ScreenWidth, ScreenHeight]} />
            <Text fontSize={16} text="Bye!" bounds={[37, 0, 100, 16]} font="PoppinsLight_8" />
        </View>
    );
}
