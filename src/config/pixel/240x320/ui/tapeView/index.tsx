import * as React from '@/libs/react';

import { Tape } from '@/libs/components/Tape';
import { Text } from '@/libs/components/Text';
import { View } from '@/libs/components/View';
import { ValueBpm } from '../components/ValueBpm';
import { TextGridTape, TextGridTapeShifted } from './TextGrid';

export type Props = {
    name: string;
};

export function TapeView({ name }: Props) {
    return (
        <View name={name}>
            <Text text={`view: ${name}`} position={[0, 0, 240, 240]} />

            <Tape position={[0, 16, 240, 240]} filename="rec" />

            <ValueBpm />

            <TextGridTape />
            <TextGridTapeShifted />
        </View>
    );
}
