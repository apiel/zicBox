import * as React from '@/libs/react';

import { View } from '@/libs/nativeComponents/View';

import { GraphEncoder } from '@/libs/nativeComponents/GraphEncoder';
import { Common } from '../components/Common';
import { BassTrack, ColorTrack2, encBottomLeft, encTopLeft, encTopRight } from '../constants';
import { topValues } from '../constantsValue';
import { TextGridBass } from './TextGridBass';

export type Props = {
    name: string;
};

export function BassWaveformView({ name }: Props) {
    const pos = [...topValues];
    pos[1] += pos[3] * 0.5;
    return (
        <View name={name}>
            {/* <KnobValue
                value="Bass STEP_FREQ"
                bounds={topLeftKnob}
                encoder_id={encTopLeft}
                COLOR="tertiary"
                track={BassTrack}
            />
            <KnobValue
                value="Bass STAIRCASE"
                bounds={topRightKnob}
                encoder_id={encTopRight}
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
                encoders={[`${encTopLeft} WAVEFORM_TYPE`, `${encTopRight} SHAPE`, `${encBottomLeft} MACRO`]}
            />
            {/* <KnobValue
                value="Bass NOISE"
                bounds={bottomLeftKnob}
                encoder_id={encBottomLeft}
                COLOR="quaternary"
                track={BassTrack}
            /> */}
            {/* <KnobValue
                value="Bass GAIN_CLIPPING"
                bounds={bottomRightKnob}
                encoder_id={encBottomRight}
                COLOR="secondary"
                track={BassTrack}
            /> */}

            <TextGridBass selected={'Wave'} viewName={name} />
            <Common selected={'Bass'} track={BassTrack} selectedBackground={ColorTrack2} />
        </View>
    );
}
