import * as React from '@/libs/react';

import { Value } from '@/libs/components/Value';
import { View } from '@/libs/components/View';
import {
    bottomLeft,
    bottomRight,
    primary,
    quaternary,
    secondary,
    tertiary,
    topLeft,
    topRight,
} from '../constantsValue';
import { TextGridCommon } from '../TextGridCommon';
import { TextGridPage1 } from './TextGridPage1';

export type Props = {
    name: string;
};

export function DistortionView({ name }: Props) {
    return (
        <View name={name}>
            <Value value="Distortion WAVESHAPE" position={topLeft} encoder_id={0} {...tertiary} />
            <Value value="Distortion DRIVE" position={bottomLeft} encoder_id={1} {...primary} />
            <Value value="Distortion COMPRESS" position={topRight} encoder_id={2} {...secondary} />
            <Value value="Distortion BASS" position={bottomRight} encoder_id={3} {...quaternary} />
            <TextGridPage1 />
            <TextGridCommon selected={0} />
        </View>
    );
}
