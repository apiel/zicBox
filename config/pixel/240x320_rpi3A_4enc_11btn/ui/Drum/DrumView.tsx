import * as React from '@/libs/react';

import { SeqView } from '../components/SeqView';
import { AmpView } from './AmpView';
import { ClickView } from './ClickView';
import { FrequencyView } from './FrequencyView';
import { FxView } from './FxView';
import { MainView } from './MainView';
import { ViewSelector } from './ViewSelector';
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
            <FxView name={`${synthName}Fx`} track={track} synthName={synthName} color={color} />
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
            <ClickView
                name={`${synthName}Click`}
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
                    <ViewSelector
                        selected={'Seq.'}
                        color={color}
                        synthName={synthName}
                        viewName={`${synthName}Sequencer`}
                        hideTitle
                    />
                }
            />
        </>
    );
}
