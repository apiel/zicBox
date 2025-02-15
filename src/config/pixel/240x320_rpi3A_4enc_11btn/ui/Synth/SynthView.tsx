import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
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
                bounds={topLeftKnob}
                encoder_id={encTopLeft}
                COLOR="tertiary"
                track={SynthTrack}
            />
            <KnobValue
                value="Synth OSC_MIX"
                bounds={topRightKnob}
                encoder_id={encTopRight}
                COLOR="primary"
                track={SynthTrack}
            />
            <KnobValue
                value="Synth FM_AMOUNT"
                bounds={bottomLeftKnob}
                encoder_id={encBottomLeft}
                COLOR="quaternary"
                track={SynthTrack}
            />
            {/* <KnobValue
                value="Synth MOD_INDEX"
                bounds={bottomRightKnob}
                encoder_id={encBottomRight}
                COLOR="secondary"
                track={SynthTrack}
            /> */}

            <TextGridSynth selected={'Main'} viewName={name} />
            <Common selected={'Synth'} track={SynthTrack} selectedBackground={ColorTrack4} />
        </View>
    );
}
