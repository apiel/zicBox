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
                audioPlugin="Bass" param="STEP_FREQ"
                bounds={topLeftKnob}
                encoderId={encTopLeft}
                color="tertiary"
                track={BassTrack}
            />
            <KnobValue
                audioPlugin="Bass" param="STAIRCASE"
                bounds={topRightKnob}
                encoderId={encTopRight}
                color="primary"
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
                audioPlugin="Bass" param="NOISE"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="quaternary"
                track={BassTrack}
            /> */}
            {/* <KnobValue
                audioPlugin="Bass" param="GAIN_CLIPPING"
                bounds={bottomRightKnob}
                encoderId={encBottomRight}
                color="secondary"
                track={BassTrack}
            /> */}

            <TextGridBass selected={'Wave'} viewName={name} />
            <Common selected={'Bass'} track={BassTrack} selectedBackground={ColorTrack2} />
        </View>
    );
}
