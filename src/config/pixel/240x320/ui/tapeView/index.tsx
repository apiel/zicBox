import * as React from '@/libs/react';

import { Tape } from '@/libs/components/Tape';
import { View } from '@/libs/components/View';
import { ValueBpm } from '../components/ValueBpm';
import { TextGridTape, TextGridTapeShifted } from './TextGrid';

export type Props = {
    name: string;
};

export function TapeView({ name }: Props) {
    return (
        <View name={name}>
            <Tape position={[0, 0, 240, 280]} filename="rec" />

            <ValueBpm />

            <TextGridTape />
            <TextGridTapeShifted />
        </View>
    );
}
