import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { PercTrack } from '../constants';
import { bottomLeftKnob, bottomRightKnob, topLeftKnob, topRightKnob } from '../constantsValue';
import { TextGridPerc } from './TextGridPerc';

export type Props = {
    name: string;
};

export function PercView({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="Volume VOLUME"
                position={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
                track={PercTrack}
            />
            <KnobValue
                value="Perc DURATION"
                position={topRightKnob}
                encoder_id={2}
                COLOR="secondary"
                track={PercTrack}
            />
            <KnobValue
                value="Perc BASE_FREQ"
                position={bottomLeftKnob}
                encoder_id={1}
                track={PercTrack}
            />
            <KnobValue
                value="Perc OVERTONES"
                position={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
                track={PercTrack}
            />

            <TextGridPerc selected={1} viewName={name} />
            <Common selected={1} track={PercTrack} />
        </View>
    );
}
