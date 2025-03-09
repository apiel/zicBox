import * as React from '@/libs/react';

import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import { ColorTrack4, SynthTrack } from '../constants';
import { TextGridSynth } from './TextGridSynth';

export type Props = {
    name: string;
};

export function SynthEnv2View({ name }: Props) {
    return (
        <View name={name}>
            {/* <Adsr
                bounds={halfFullValues}
                audioPlugin="Synth"
                values={['ATTACK_2', 'DECAY_2', 'SUSTAIN_2', 'RELEASE_2']}
            /> */}

            <TextGridSynth selected={'Env2'} viewName={name} />
            <Common selected={'Synth'} track={SynthTrack} selectedBackground={ColorTrack4} />
        </View>
    );
}
