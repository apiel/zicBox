import * as React from '@/libs/react';

import { SeqView } from '../components/SeqView';
import { AmpView } from './AmpView';
import { ClickView } from './ClickView';
import { DistortionView } from './DistortionView';
import { FrequencyView } from './FrequencyView';
import { Drum23MainView } from './MainView';
import { TextGridDrum23 } from './TextGridDrum23';
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
            <DistortionView
                name={`${synthName}Distortion`}
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
                textGrid={(<TextGridDrum23 selected={'Seq.'} color={color} synthName={synthName} hideTitle />)}
            />
        </>
    );
}
