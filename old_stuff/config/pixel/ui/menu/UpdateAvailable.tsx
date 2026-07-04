import * as React from '@/libs/react';

import { Pixels } from '@/libs/nativeComponents/Pixels';
import { Text } from '@/libs/nativeComponents/Text';
import { View } from '@/libs/nativeComponents/View';
import { C2, C3, menuTextColor, ScreenHeight, ScreenWidth, W1_4, W2_4 } from '../constants';

export type Props = {
    name: string;
};

export function UpdateAvailable({ name }: Props) {
    return (
        <View name={name}>
            <Pixels bounds={[0, 0, ScreenWidth, ScreenHeight]} color="#999999" />
            <Text
                fontSize={24}
                text="Update Available"
                bounds={[0, 60, ScreenWidth, 16]}
                centered
                font="PoppinsLight_12"
            />

            <Text
                text="Exit"
                bounds={[W1_4, ScreenHeight - 20, W1_4, 16]}
                centered={true}
                color={menuTextColor}
                keys={[{ key: C2, action: 'setView:&previous' }]}
            />
            <Text
                text="Update"
                bounds={[W2_4, ScreenHeight - 20, W1_4, 16]}
                centered={true}
                // color={menuTextColor}
                bgColor="#4cb663ff"
                keys={[{ key: C3, action: 'setView:Updating', action2: `sh:/opt/scripts/update.sh &` }]}
            />
        </View>
    );
}
