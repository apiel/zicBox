import * as React from '@/libs/react';

import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { TextGridSeq } from './TextGridSeq';

export type Props = {
    name: string;
};

export function SeqView({ name }: Props) {
    return (
        <View name={name}>

            <TextGridSeq />
            <Common selected={0} />
        </View>
    );
}
