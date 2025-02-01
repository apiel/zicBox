import * as React from '@/libs/react';

import { Adsr } from '@/libs/components/Adsr';
import { View } from '@/libs/components/View';
import { Common } from '../components/Common';
import { ColorTrack4, SynthTrack } from '../constants';
import { halfFullValues } from '../constantsValue';
import { TextGridSynth } from './TextGridSynth';

export type Props = {
    name: string;
};

export function SynthEnv2View({ name }: Props) {
    return (
        <View name={name}>
            <Adsr position={halfFullValues} plugin="Synth" values="ATTACK_2 DECAY_2 SUSTAIN_2 RELEASE_2" />

            <TextGridSynth selected={'Env1/2'} viewName={name} />
            <Common selected={'Synth'} track={SynthTrack} selectedBackground={ColorTrack4} />
        </View>
    );
}
