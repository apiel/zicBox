import * as React from '@/libs/react';

import { AmpView } from './AmpView';
import { ClickView } from './ClickView';
import { DistortionView } from './DistortionView';
import { Drum23MainView } from './Drum23MainView';
import { FrequencyView } from './FrequencyView';
import { Drum23SeqView } from './SeqView';
import { WaveformView } from './WaveformView';

export type Props = {
    track: number;
    synthName: string;
    color: string;
};

export function Drum23View({ track, synthName, color }: Props) {
    return (
        <>
            <Drum23MainView name={synthName} track={track} synthName={synthName} color={color} />
            <DistortionView name={`${synthName}Distortion`} track={track} synthName={synthName} color={color} />
            <WaveformView name={`${synthName}Waveform`} track={track} synthName={synthName} color={color} />
            <FrequencyView name={`${synthName}Frequency`} track={track} synthName={synthName} color={color} />
            <AmpView name={`${synthName}Amplitude`} track={track} synthName={synthName} color={color} />
            <ClickView name={`${synthName}Click`} track={track} synthName={synthName} color={color} />
            <Drum23SeqView name={`${synthName}Sequencer`} track={track} synthName={synthName} color={color} />
        </>
    );
}
