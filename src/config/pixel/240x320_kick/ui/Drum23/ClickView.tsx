import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import {
    bottomLeftKnob,
    bottomRightKnob,
    topLeftKnob,
    topRightKnob
} from '../constantsValue';
import { TextGridDrum23 } from './TextGridDrum23';

export type Props = {
    name: string;
};

export function ClickView({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="Drum23 CLICK"
                position={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
            />
            <KnobValue
                value="Drum23 CLICK_CUTOFF"
                position={topRightKnob}
                encoder_id={2}
                COLOR="primary"
                TYPE="STRING"
                FONT_VALUE_SIZE={12}
            />
            <KnobValue
                value="Drum23 CLICK_DURATION"
                position={bottomLeftKnob}
                encoder_id={1}
                COLOR="quaternary"
            />
            <KnobValue
                value="Drum23 CLICK_RESONANCE"
                position={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
            />

            <TextGridDrum23 selected={3} viewName={name} />
            <Common selected={1} />
        </View>
    );
}
