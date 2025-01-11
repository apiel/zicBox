import * as React from '@/libs/react';

import { View } from '@/libs/components/View';
import { Common } from '../Common';
import { TextGridSeq } from './TextGridSeq';

export type Props = {
    name: string;
};

export function SeqView({ name }: Props) {
    return (
        <View name={name}>

            <TextGridSeq />
            <Common selected={3} />
        </View>
    );
}
