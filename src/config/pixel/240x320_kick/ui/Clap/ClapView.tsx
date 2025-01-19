import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { ClapTrack } from '../constants';
import { bottomLeftKnob, bottomRightKnob, topLeftKnob, topRightKnob } from '../constantsValue';
import { TextGridClap } from './TextGridClap';

export type Props = {
    name: string;
};

export function ClapView({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="Volume VOLUME"
                position={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
                track={ClapTrack}
            />
            <KnobValue
                value="Clap CARRIER_FREQ"
                position={topRightKnob}
                encoder_id={2}
                COLOR="secondary"
                track={ClapTrack}
            />
            <KnobValue
                value="Clap MOD_FREQ"
                position={bottomLeftKnob}
                encoder_id={1}
                track={ClapTrack}
            />
            <KnobValue
                value="Clap MOD_INDEX"
                position={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
                track={ClapTrack}
            />

            <TextGridClap selected={1} viewName={name} />
            <Common selected={1} track={ClapTrack} />
        </View>
    );
}
