import * as React from '@/libs/react';

import { DrumEnvelop } from '@/libs/components/DrumEnvelop';
import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { ColorTrack1, Drum23Track } from '../constants';
import { bottomRightKnob, topValues } from '../constantsValue';
import { TextGridDrum23 } from './TextGridDrum23';

export type Props = {
    name: string;
};

export function AmpView({ name }: Props) {
    return (
        <View name={name}>
             <DrumEnvelop
                 position={topValues}
                 plugin="Drum23"
                 envelop_data_id="0"
                 RENDER_TITLE_ON_TOP={false}
                 encoder_time={0}
                 encoder_phase={2}
                 encoder_modulation={1}
                 track={Drum23Track}
             />
            <KnobValue
                position={bottomRightKnob}
                value="Drum23 DURATION"
                encoder_id={3}
                color="quaternary"
                track={Drum23Track}
            />
            <TextGridDrum23 selected={4} viewName={name} />
            <Common selected={0} track={Drum23Track} selectedBackground={ColorTrack1} />
        </View>
    );
}
