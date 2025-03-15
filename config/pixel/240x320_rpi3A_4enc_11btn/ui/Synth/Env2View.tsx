import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import { ColorTrack4, encBottomLeft, encTopLeft, encTopRight, SynthTrack } from '../constants';
import { bottomLeftKnob, topLeftKnob, topRightKnob } from '../constantsValue';
import { TextGridSynth } from './TextGridSynth';

export type Props = {
    name: string;
};

export function SynthEnv2View({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                audioPlugin="Synth"
                param="CUTOFF_MOD"
                bounds={topLeftKnob}
                encoderId={encTopLeft}
                color="tertiary"
                track={SynthTrack}
            />
            <KnobValue
                audioPlugin="Synth"
                param="RESONANCE_MOD"
                bounds={topRightKnob}
                encoderId={encTopRight}
                color="primary"
                track={SynthTrack}
            />
            <KnobValue
                audioPlugin="Synth"
                param="FREQUENCY_MOD"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="quaternary"
                track={SynthTrack}
            />
            {/* <KnobValue
                            audioPlugin="Synth" param="OSC_MOD"
                            bounds={bottomRightKnob}
                            encoderId={encBottomRight}
                            color="secondary"
                            track={SynthTrack}
                        /> */}

            <TextGridSynth selected={'Mod.'} viewName={name} />
            <Common selected={'Synth'} track={SynthTrack} selectedBackground={ColorTrack4} />
        </View>
    );
}
