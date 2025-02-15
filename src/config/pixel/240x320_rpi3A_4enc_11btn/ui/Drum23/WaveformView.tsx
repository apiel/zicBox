import * as React from '@/libs/react';

import { GraphEncoder } from '@/libs/nativeComponents/GraphEncoder';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
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
                bounds={topValues}
                plugin="Drum23"
                data_id="WAVEFORM"
                RENDER_TITLE_ON_TOP={false}
                encoders={[`${encTopLeft} WAVEFORM_TYPE`, `${encTopRight} SHAPE`, `${encBottomLeft} MACRO`]}
                track={Drum23Track}
            />
            <KnobValue
                bounds={bottomRightKnob}
                audioPlugin="Drum23" param="PITCH"
                encoderId={encBottomRight}
                color="secondary"
                track={Drum23Track}
            />
            <TextGridDrum23 selected={'Wave'} viewName={name} />
            <Common selected={'Kick'} track={Drum23Track} selectedBackground={ColorTrack1} />
        </View>
    );
}
