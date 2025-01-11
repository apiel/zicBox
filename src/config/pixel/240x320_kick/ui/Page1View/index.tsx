import * as React from '@/libs/react';

import { Value } from '@/libs/components/Value';
import { View } from '@/libs/components/View';
import { bottomLeft, bottomRight, primary, quaternary, tertiary, topLeft, topRight } from '../constantsValue';
import { TextGridCommon } from '../TextGridCommon';
import { TextGridMain } from './TextGrid';

export type Props = {
    name: string;
};

export function Page1View({ name }: Props) {
    return (
        <View name={name}>
            <Value
                value="Volume VOLUME"
                position={topLeft}
                encoder_id={0}
                {...tertiary}
            />
            <Value
                value="Drum23 GAIN_CLIPPING"
                position={bottomLeft}
                encoder_id={1}
                {...primary}
            />
            <Value
                value="MMFilter CUTOFF"
                position={topRight}
                encoder_id={2}
                {...quaternary}
                USE_STRING_VALUE
            />
            <Value
                value="MMFilter RESONANCE"
                position={bottomRight}
                encoder_id={3}
                {...quaternary}
            />
            <TextGridMain />
            <TextGridCommon selected={0} />
        </View>
    );
}
