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

export function Sample2View({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="Sample LOOP_POSITION"
                bounds={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
                track={SampleTrack}
            />
            <KnobValue
                value="Sample LOOP_LENGTH"
                bounds={topRightKnob}
                encoder_id={2}
                COLOR="tertiary"
                track={SampleTrack}
            />
            <KnobValue
                value="Sample LOOP_RELEASE"
                bounds={bottomLeftKnob}
                encoder_id={1}
                COLOR="tertiary"
                track={SampleTrack}
            />
            <KnobValue
                value="TrackFx REVERB"
                bounds={bottomRightKnob}
                encoder_id={3}
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
