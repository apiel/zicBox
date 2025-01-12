import * as React from '@/libs/react';

import { Value } from '@/libs/components/Value';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
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
import { TextGridSynth } from './TextGridSynth';

export type Props = {
    name: string;
};

export function ClickView({ name }: Props) {
    return (
        <View name={name}>
            <Value value="Drum23 CLICK" position={topLeft} encoder_id={0} {...tertiary} />
            <Value
                value="Drum23 CLICK_DURATION"
                position={bottomLeft}
                encoder_id={1}
                {...primary}
            />
            <Value
                value="Drum23 CLICK_CUTOFF"
                position={topRight}
                encoder_id={2}
                {...secondary}
                USE_STRING_VALUE
            />
            <Value
                value="Drum23 CLICK_RESONANCE"
                position={bottomRight}
                encoder_id={3}
                {...quaternary}
            />
            <TextGridSynth selected={3} viewName={name} />
            <Common selected={1} />
        </View>
    );
}
