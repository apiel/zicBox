import * as React from '@/libs/react';

import { GraphEncoder } from '@/libs/components/GraphEncoder';
import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { ColorTrack1, Drum23Track, encBottomLeft, encBottomRight, encTopLeft, encTopRight } from '../constants';
import { bottomRightKnob, topValues } from '../constantsValue';
import { TextGridDrum23 } from './TextGridDrum23';

export type Props = {
    name: string;
};

export function WaveformView({ name }: Props) {
    return (
        <View name={name}>
            <GraphEncoder
                position={topValues}
                plugin="Drum23"
                data_id="WAVEFORM"
                RENDER_TITLE_ON_TOP={false}
                encoders={[`${encTopLeft} WAVEFORM_TYPE`, `${encTopRight} SHAPE`, `${encBottomLeft} MACRO`]}
                track={Drum23Track}
            />
            <KnobValue
                position={bottomRightKnob}
                value="Drum23 PITCH"
                encoder_id={encBottomRight}
                color="secondary"
                track={Drum23Track}
            />
            <TextGridDrum23 selected={'Wave'} viewName={name} />
            <Common selected={'Kick'} track={Drum23Track} selectedBackground={ColorTrack1} />
        </View>
    );
}
