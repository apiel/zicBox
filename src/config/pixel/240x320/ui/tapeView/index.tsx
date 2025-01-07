import * as React from '@/libs/react';

import { Text } from '@/libs/components/Text';
import { View } from '@/libs/components/View';

export type Props = {
    name: string;
};

export function TapeView({ name }: Props) {
    return <View name={name}>
        <Text text={`view: ${name}`} position={[0, 0, 240, 240]} />
    </View>;
}
