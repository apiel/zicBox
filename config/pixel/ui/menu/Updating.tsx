import * as React from '@/libs/react';

import { Pixels } from '@/libs/nativeComponents/Pixels';
import { Text } from '@/libs/nativeComponents/Text';
import { View } from '@/libs/nativeComponents/View';
import { ScreenHeight, ScreenWidth } from '../constants';

export type Props = {
    name: string;
};

export function Updating({ name }: Props) {
    return (
        <View name={name}>
            <Pixels bounds={[0, 0, ScreenWidth, ScreenHeight]} color='#999999' />
            <Text fontSize={32} text="Updating!" bounds={[40, 60, 200, 16]} font="PoppinsLight_16" />
        </View>
    );
}
