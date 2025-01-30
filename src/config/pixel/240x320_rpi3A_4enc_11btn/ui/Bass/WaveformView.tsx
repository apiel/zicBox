import * as React from '@/libs/react';

import { View } from '@/libs/components/View';

import { GraphEncoder } from '@/libs/components/GraphEncoder';
import { Common } from '../components/Common';
import { BassTrack, ColorTrack2, encBottomLeft, encTopLeft, encTopRight } from '../constants';
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
                position={topLeftKnob}
                encoder_id={encTopLeft}
                COLOR="tertiary"
                track={BassTrack}
            />
            <KnobValue
                value="Bass STAIRCASE"
                position={topRightKnob}
                encoder_id={encTopRight}
                COLOR="primary"
                track={BassTrack}
            /> */}
            <GraphEncoder
                // position={topValues}
                position={pos}
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
                position={bottomLeftKnob}
                encoder_id={encBottomLeft}
                COLOR="quaternary"
                track={BassTrack}
            /> */}
            {/* <KnobValue
                value="Bass GAIN_CLIPPING"
                position={bottomRightKnob}
                encoder_id={encBottomRight}
                COLOR="secondary"
                track={BassTrack}
            /> */}

            <TextGridBass selected={4} viewName={name} />
            <Common selected={3} track={BassTrack} selectedBackground={ColorTrack2} />
        </View>
    );
}
