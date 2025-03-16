import * as React from '@/libs/react';
import { SeqView } from '../components/SeqView';
import { FxView } from './FxView';
import { MainView } from './Main';
import { ViewSelector } from './ViewSelector';
import { WaveView } from './Wave';
import { Wave2View } from './Wave2';

export type Props = {
    track: number;
    synthName: string;
    color: string;
};

export function SampleView({ track, synthName, color }: Props) {
    return (
        <>
            <MainView name={synthName} track={track} synthName={synthName} color={color} />
            <WaveView name={`${synthName}Wave`} track={track} synthName={synthName} color={color} />
            <Wave2View name={`${synthName}Browse`} track={track} synthName={synthName} color={color} />
            <FxView name={`${synthName}Fx`} track={track} synthName={synthName} color={color} />
            {/* <Env1View name={`${synthName}Env1`} track={track} synthName={synthName} color={color} /> */}
            {/* <Env2View name={`${synthName}Env2`} track={track} synthName={synthName} color={color} /> */}
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
