import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { Sample } from '@/libs/components/Sample';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { TextGridDrums } from '../components/TextGridDrums';
import { SampleTrack, ScreenWidth } from '../constants';
import { bottomLeftKnob, bottomRightKnob, topLeftKnob, topRightKnob } from '../constantsValue';

export type Props = {
    name: string;
};

export function SampleView({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="TrackFx VOLUME_DRIVE"
                position={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
                track={SampleTrack}
            />
            <KnobValue
                value="Sample BROWSER"
                position={topRightKnob}
                encoder_id={2}
                track={SampleTrack}
                STRING_VALUE_REPLACE_TITLE
            />
            <KnobValue
                value="Sample START"
                position={bottomLeftKnob}
                encoder_id={1}
                COLOR="secondary"
                track={SampleTrack}
            />
            <KnobValue
                value="Sample END"
                position={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
                track={SampleTrack}
            />

            <Sample
                position={[0, 245, ScreenWidth, 50]}
                track={SampleTrack}
                plugin="Sample SAMPLE_BUFFER SAMPLE_INDEX"
                loop_points_color={'tertiary'}
                BACKGROUND_COLOR="background"
            />

            <TextGridDrums selected={1} viewName={name} target="Sample" />
            <Common selected={1} track={SampleTrack} />
        </View>
    );
}
