import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { Drum1, TracksSelector } from '../components/Common';
import { enc1, enc2, enc3, enc4, enc5, enc6, enc7, enc8, enc9 } from '../constantsValue';
import { DrumLayout } from './Layout';
import { ViewSelector } from './ViewSelector';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function MainView({ name, track, synthName, color }: Props) {
    return (
        <DrumLayout
            viewName={name}
            color={color}
            synthName={synthName}
            content={
                <>
                    <KnobValue
                        audioPlugin="TrackFx"
                        param="VOLUME"
                        {...enc1}
                        color="tertiary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin="MMFilter"
                        param="CUTOFF"
                        {...enc2}
                        color="secondary"
                        type="STRING"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin="MMFilter"
                        param="RESONANCE"
                        {...enc3}
                        color="secondary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin="Distortion"
                        param="WAVESHAPE"
                        {...enc4}
                        color="tertiary"
                        track={track}
                    />

                    <KnobValue
                        audioPlugin="Distortion"
                        param="COMPRESS"
                        {...enc5}
                        color="primary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin="Distortion"
                        param="DRIVE"
                        {...enc6}
                        color="quaternary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin="Distortion"
                        param="BASS"
                        {...enc7}
                        color="secondary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="HIGH_FREQ_BOOST"
                        {...enc8}
                        color="tertiary"
                        track={track}
                    />

                    <KnobValue
                        audioPlugin={synthName}
                        param="GAIN_CLIPPING"
                        {...enc9}
                        color="primary"
                        track={track}
                    />
                    <ViewSelector color={color} synthName={synthName} viewName={name} />
                    <TracksSelector
                        selectedBackground={color}
                        selected={Drum1}
                        viewName={name}
                        trackName={Drum1}
                        pageCount={2}
                        currentPage={1}
                    />
                </>
            }
        />
    );
}
