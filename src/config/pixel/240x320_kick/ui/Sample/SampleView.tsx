import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { TextGridDrums } from '../components/TextGridDrums';
import { SampleTrack } from '../constants';
import { bottomLeftKnob, topLeftKnob, topRightKnob } from '../constantsValue';

export type Props = {
    name: string;
};

export function SampleView({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="Volume VOLUME"
                position={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
                track={SampleTrack}
            />
            <KnobValue
                value="Sample BROWSER"
                position={topRightKnob}
                encoder_id={2}
                COLOR="secondary"
                track={SampleTrack}
                STRING_VALUE_REPLACE_TITLE
            />
            <KnobValue
                value="Sample BROWSER"
                position={bottomLeftKnob}
                encoder_id={1}
                track={SampleTrack}
            />
            {/* <KnobValue
                value="Sample TONE_DECAY"
                position={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
                track={SampleTrack}
            /> */}

            <TextGridDrums selected={1} viewName={name} target="Sample" />
            <Common selected={1} track={SampleTrack} />
        </View>
    );
}
