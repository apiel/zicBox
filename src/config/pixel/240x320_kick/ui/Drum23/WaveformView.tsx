import * as React from '@/libs/react';

import { GraphEncoder } from '@/libs/components/GraphEncoder';
import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { ColorTrack1, Drum23Track } from '../constants';
import { bottomRightKnob, topValues } from '../constantsValue';
import { TextGridDrum23 } from './TextGridDrum23';

export type Props = {
    name: string;
};

export function WaveformView({ name }: Props) {
    return (
        <View name={name}>
            <GraphEncoder
                bounds={topValues}
                plugin="Drum23"
                data_id="WAVEFORM"
                RENDER_TITLE_ON_TOP={false}
                encoders={['0 WAVEFORM_TYPE', '1 SHAPE', '2 MACRO']}
                track={Drum23Track}
            />
            <KnobValue
                bounds={bottomRightKnob}
                value="Drum23 PITCH"
                encoder_id={3}
                color="secondary"
                track={Drum23Track}
            />
            <TextGridDrum23 selected={1} viewName={name} />
            <Common selected={0} track={Drum23Track} selectedBackground={ColorTrack1} />
        </View>
    );
}
