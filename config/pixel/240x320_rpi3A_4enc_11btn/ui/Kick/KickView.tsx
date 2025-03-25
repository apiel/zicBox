import * as React from '@/libs/react';

import { SeqView } from '../components/SeqView';
import { AmpView } from './AmpView';
import { ClickView } from './ClickView';
import { DistortionView } from './DistortionView';
import { FrequencyView } from './FrequencyView';
import { MainView } from './MainView';
import { TextGridKick } from './TextGridKick';
import { WaveformView } from './WaveformView';

export type Props = {
    track: number;
    synthName: string;
    color: string;
};

export function KickView({ track, synthName, color }: Props) {
    return (
        <>
            <MainView name={synthName} track={track} synthName={synthName} color={color} />
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
                maxStepLen={1}
                textGrid={(<TextGridKick selected={'Seq.'} color={color} synthName={synthName} hideTitle />)}
            />
        </>
    );
}
