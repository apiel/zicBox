import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { Sample } from '@/libs/nativeComponents/Sample';
import { View } from '@/libs/nativeComponents/View';
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
                value="TrackFx VOLUME"
                bounds={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
                track={SampleTrack}
            />
            <KnobValue
                value="Sample BROWSER"
                bounds={topRightKnob}
                encoder_id={2}
                track={SampleTrack}
                STRING_VALUE_REPLACE_TITLE
            />
            <KnobValue
                value="Sample START"
                bounds={bottomLeftKnob}
                encoder_id={1}
                COLOR="secondary"
                track={SampleTrack}
            />
            <KnobValue
                value="Sample END"
                bounds={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
                track={SampleTrack}
            />

            <Sample
                bounds={[0, 245, ScreenWidth, 50]}
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
