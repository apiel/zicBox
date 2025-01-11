import * as React from '@/libs/react';

import { View } from '@/libs/components/View';
import { TextGridTape } from './TextGrid';

export type Props = {
    name: string;
};

export function KickView({ name }: Props) {
    return (
        <View name={name}>

            <TextGridTape />
        </View>
    );
}
