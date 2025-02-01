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

export function SynthEnv1View({ name }: Props) {
    return (
        <View name={name}>
            <Adsr position={halfFullValues} plugin="Synth" values="ATTACK_1 DECAY_1 SUSTAIN_1 RELEASE_1" />

            <TextGridSynth selected={'Env1'} viewName={name} />
            <Common selected={'Synth'} track={SynthTrack} selectedBackground={ColorTrack4} />
        </View>
    );
}
