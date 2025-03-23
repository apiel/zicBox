import * as React from '@/libs/react';
import { SeqView } from '../components/SeqView';
import { EnvLfoView } from './EnvLfoView';
import { EnvModView } from './EnvModView';
import { FxView } from './FxView';
import { LfoModView } from './LfoModView';
import { MainView } from './Main';
import { ViewSelector } from './ViewSelector';
import { WaveView } from './Wave';

export type Props = {
    track: number;
    synthName: string;
    color: string;
};

export function SynthView({ track, synthName, color }: Props) {
    return (
        <>
            <MainView name={synthName} track={track} synthName={synthName} color={color} />
            <WaveView name={`${synthName}Wave`} track={track} synthName={synthName} color={color} />
            <FxView name={`${synthName}Fx`} track={track} synthName={synthName} color={color} />
            <EnvLfoView name={`${synthName}EnvLfo`} track={track} synthName={synthName} color={color} />
            <EnvModView name={`${synthName}EnvMod`} track={track} synthName={synthName} color={color} />
            <LfoModView name={`${synthName}LfoMod`} track={track} synthName={synthName} color={color} />
            <SeqView
                name={`${synthName}Seq`}
                track={track}
                synthName={synthName}
                color={color}
                textGrid={
                    <ViewSelector
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
