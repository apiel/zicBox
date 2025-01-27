import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';

import { GraphEncoder } from '@/libs/components/GraphEncoder';
import { Common } from '../components/Common';
import { BassTrack, ColorTrack2 } from '../constants';
import { bottomLeftKnob, bottomRightKnob, topValues } from '../constantsValue';
import { TextGridBass } from './TextGridBass';

export type Props = {
    name: string;
};

export function BassWaveformView({ name }: Props) {
    return (
        <View name={name}>
            {/* <KnobValue
                value="Bass STEP_FREQ"
                position={topLeftKnob}
                encoder_id={0}
                COLOR="tertiary"
                track={BassTrack}
            />
            <KnobValue
                value="Bass STAIRCASE"
                position={topRightKnob}
                encoder_id={2}
                COLOR="primary"
                track={BassTrack}
            /> */}
            <GraphEncoder
                position={topValues}
                plugin="Bass"
                data_id="WAVEFORM"
                // RENDER_TITLE_ON_TOP={false}
                encoders={['0 MORPH', '2 STAIRCASE']}
                track={BassTrack}
                is_array
            />
            <KnobValue
                value="Bass NOISE"
                position={bottomLeftKnob}
                encoder_id={1}
                COLOR="quaternary"
                track={BassTrack}
            />
            <KnobValue
                value="Bass GAIN_CLIPPING"
                position={bottomRightKnob}
                encoder_id={3}
                COLOR="secondary"
                track={BassTrack}
            />

            <TextGridBass selected={4} viewName={name} />
            <Common selected={3} track={BassTrack} selectedBackground={ColorTrack2} />
        </View>
    );
}
