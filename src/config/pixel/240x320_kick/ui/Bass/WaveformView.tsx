import * as React from '@/libs/react';

import { View } from '@/libs/components/View';

import { GraphEncoder } from '@/libs/components/GraphEncoder';
import { Common } from '../components/Common';
import { BassTrack, ColorTrack2 } from '../constants';
import { topValues } from '../constantsValue';
import { TextGridBass } from './TextGridBass';

export type Props = {
    name: string;
};

export function BassWaveformView({ name }: Props) {
    const pos = topValues;
    pos[1] += pos[3] * 0.5;
    return (
        <View name={name}>
            {/* <KnobValue
                value="Bass STEP_FREQ"
                bounds={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
                track={BassTrack}
            />
            <KnobValue
                value="Bass STAIRCASE"
                bounds={topRightKnob}
                encoder_id={2}
                COLOR="primary"
                track={BassTrack}
            /> */}
            <GraphEncoder
                // bounds={topValues}
                bounds={pos}
                plugin="Bass"
                data_id="WAVEFORM"
                // RENDER_TITLE_ON_TOP={false}
                // encoders={['0 MORPH', '2 STAIRCASE']}
                // is_array
                track={BassTrack}
                RENDER_TITLE_ON_TOP={false}
                encoders={['0 WAVEFORM_TYPE', '1 SHAPE', '2 MACRO']}
            />
            {/* <KnobValue
                value="Bass NOISE"
                bounds={bottomLeftKnob}
                encoder_id={1}
                COLOR="quaternary"
                track={BassTrack}
            /> */}
            {/* <KnobValue
                value="Bass GAIN_CLIPPING"
                bounds={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
                track={BassTrack}
            /> */}

            <TextGridBass selected={4} viewName={name} />
            <Common selected={3} track={BassTrack} selectedBackground={ColorTrack2} />
        </View>
    );
}
