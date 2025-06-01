import * as React from '@/libs/react';

import { MainKeys } from '../components/Common';
import { SeqView } from '../components/SeqView';
import { AmpView } from './AmpView';
import { FrequencyView } from './FrequencyView';
import { FxView } from './Fx';
import { Fx2View } from './Fx2';
import { Layer2View } from './Layer2View';
import { Layer2_2View } from './Layer2_2View';
import { Main2View } from './Main2View';
import { MainView } from './MainView';
import { WaveformView } from './WaveformView';

export type Props = {
    track: number;
    synthName: string;
    color: string;
};

export function DrumView({ track, synthName, color }: Props) {
    return (
        <>
            <MainView name={synthName} track={track} synthName={synthName} color={color} />
            <Main2View
                name={`${synthName}:page2`}
                track={track}
                synthName={synthName}
                color={color}
            />
            <FxView name={`${synthName}Fx`} track={track} synthName={synthName} color={color} />
            <Fx2View
                name={`${synthName}Fx:page2`}
                track={track}
                synthName={synthName}
                color={color}
            />
            <WaveformView
                name={`${synthName}Waveform`}
                track={track}
                synthName={synthName}
                color={color}
            />
            <FrequencyView
                name={`${synthName}Frequency`}
                track={track}
                synthName={synthName}
                color={color}
            />
            <AmpView
                name={`${synthName}Amplitude`}
                track={track}
                synthName={synthName}
                color={color}
            />
            <Layer2View
                name={`${synthName}Layer2`}
                track={track}
                synthName={synthName}
                color={color}
            />
            <Layer2_2View
                name={`${synthName}Layer2:page2`}
                track={track}
                synthName={synthName}
                color={color}
            />

            <SeqView
                name={`${synthName}Sequencer`}
                track={track}
                synthName={synthName}
                color={color}
                maxStepLen={1}
                textGrid={
                    <>
                        <MainKeys synthName={synthName} forcePatchView />
                    </>
                }
            />
        </>
    );
}
