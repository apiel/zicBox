import * as React from '@/libs/react';

import { KnobValue } from '@/libs/components/KnobValue';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { ColorTrack4, encBottomLeft, encTopLeft, encTopRight, SynthTrack } from '../constants';
import {
    bottomLeftKnob,
    topLeftKnob,
    topRightKnob
} from '../constantsValue';
import { TextGridSynth } from './TextGridSynth';

export type Props = {
    name: string;
};

export function SynthView({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                value="TrackFx VOLUME"
                position={topLeftKnob}
                encoder_id={encTopLeft}
                COLOR="tertiary"
                track={SynthTrack}
            />
            <KnobValue
                value="Synth OSC_MIX"
                position={topRightKnob}
                encoder_id={encTopRight}
                COLOR="primary"
                track={SynthTrack}
            />
            <KnobValue
                value="Synth FM_AMOUNT"
                position={bottomLeftKnob}
                encoder_id={encBottomLeft}
                COLOR="quaternary"
                track={SynthTrack}
            />
            {/* <KnobValue
                value="Synth MOD_INDEX"
                position={bottomRightKnob}
                encoder_id={encBottomRight}
                COLOR="secondary"
                track={SynthTrack}
            /> */}

            <TextGridSynth selected={'Main'} viewName={name} />
            <Common selected={'Synth'} track={SynthTrack} selectedBackground={ColorTrack4} />
        </View>
    );
}
