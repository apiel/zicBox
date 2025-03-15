import * as React from '@/libs/react';
import { SeqView } from '../components/SeqView';
import { SynthEnv1View } from './Env1View';
import { SynthEnv2View } from './Env2View';
import { SynthFilterView } from './FilterView';
import { SynthFxView } from './FxView';
import { SynthMainView } from './Main';
import { TextGridSynth } from './TextGridSynth';

export type Props = {
    track: number;
    synthName: string;
    color: string;
};

export function SynthView({ track, synthName, color }: Props) {
    return (
        <>
            <SynthMainView name={synthName} track={track} synthName={synthName} color={color} />
            <SynthFxView
                name={`${synthName}Fx`}
                track={track}
                synthName={synthName}
                color={color}
            />
            <SynthFilterView
                name={`${synthName}Filter`}
                track={track}
                synthName={synthName}
                color={color}
            />
            <SynthEnv1View
                name={`${synthName}Env1`}
                track={track}
                synthName={synthName}
                color={color}
            />
            <SynthEnv2View
                name={`${synthName}Env2`}
                track={track}
                synthName={synthName}
                color={color}
            />
            <SeqView
                name={`${synthName}Seq`}
                track={track}
                synthName={synthName}
                color={color}
                textGrid={
                    <TextGridSynth
                        selected={'Seq.'}
                        viewName={`${synthName}Seq`}
                        color={color}
                        synthName={synthName}
                        hideTitle
                    />
                }
            />
        </>
    );
}
