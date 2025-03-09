import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import { ColorTrack4, encBottomLeft, encBottomRight, encTopLeft, encTopRight, SynthTrack } from '../constants';
import { bottomLeftKnob, bottomRightKnob, topLeftKnob, topRightKnob } from '../constantsValue';
import { TextGridSynth } from './TextGridSynth';

export type Props = {
    name: string;
};

export function SynthEnv1View({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                audioPlugin="Synth" param="ATTACK"
                bounds={topLeftKnob}
                encoderId={encTopLeft}
                color="tertiary"
                track={SynthTrack}
            />
            <KnobValue
                audioPlugin="Synth" param="RELEASE"
                bounds={topRightKnob}
                encoderId={encTopRight}
                color="primary"
                track={SynthTrack}
            />
            <KnobValue
                audioPlugin="Synth" param="FREQUENCY_MOD"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="quaternary"
                track={SynthTrack}
            />
            <KnobValue
                audioPlugin="Synth" param="OSC_MOD"
                bounds={bottomRightKnob}
                encoderId={encBottomRight}
                color="secondary"
                track={SynthTrack}
            />

            <TextGridSynth selected={'Env1'} viewName={name} />
            <Common selected={'Synth'} track={SynthTrack} selectedBackground={ColorTrack4} />
        </View>
    );
}
