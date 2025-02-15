import * as React from '@/libs/react';

import { View } from '@/libs/nativeComponents/View';

import { GraphEncoder } from '@/libs/nativeComponents/GraphEncoder';
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
                audioPlugin="Bass" param="STEP_FREQ"
                bounds={topLeftKnob}
                encoderId={0}
                color="tertiary"
                track={BassTrack}
            />
            <KnobValue
                audioPlugin="Bass" param="STAIRCASE"
                bounds={topRightKnob}
                encoderId={2}
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
                encoders={['0 WAVEFORM_TYPE', '1 SHAPE', '2 MACRO']}
            />
            {/* <KnobValue
                audioPlugin="Bass" param="NOISE"
                bounds={bottomLeftKnob}
                encoderId={1}
                color="quaternary"
                track={BassTrack}
            /> */}
            {/* <KnobValue
                audioPlugin="Bass" param="GAIN_CLIPPING"
                bounds={bottomRightKnob}
                encoderId={3}
                color="secondary"
                track={BassTrack}
            /> */}

            <TextGridBass selected={4} viewName={name} />
            <Common selected={3} track={BassTrack} selectedBackground={ColorTrack2} />
        </View>
    );
}
