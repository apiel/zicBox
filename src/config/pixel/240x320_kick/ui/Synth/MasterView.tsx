import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { W2_4 } from '../constants';
import { TextGridSynth } from './TextGridSynth';

export type Props = {
    name: string;
};

export function MasterView({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="Volume VOLUME"
                position={[0, 50, W2_4 - 2, 80]}
                encoder_id={0}
                COLOR="tertiary"
            />
            <KnobValue
                value="MMFilter CUTOFF"
                position={[W2_4, 50, W2_4 - 2, 80]}
                encoder_id={2}
                COLOR="quaternary"
                FONT_VALUE_SIZE={8}
                TYPE="STRING"
            />
            <KnobValue
                value="Drum23 GAIN_CLIPPING"
                position={[0, 150, W2_4 - 2, 80]}
                encoder_id={1}
            />
            <KnobValue
                value="MMFilter RESONANCE"
                position={[W2_4, 150, W2_4 - 2, 80]}
                encoder_id={3}
                COLOR="quaternary"
            />

            {/* <Value value="Volume VOLUME" position={topLeft} encoder_id={0} {...tertiary} />
            <Value value="Drum23 GAIN_CLIPPING" position={bottomLeft} encoder_id={1} {...primary} />
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
            /> */}
            <TextGridSynth selected={0} viewName={name} />
            <Common selected={1} />
        </View>
    );
}
