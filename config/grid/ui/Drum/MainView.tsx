import * as React from '@/libs/react';

import { GraphEncoder } from '@/libs/nativeComponents/GraphEncoder';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { Value } from '@/libs/nativeComponents/Value';
import { Drum1, TracksSelector } from '../components/Common';
import {
    bounds5,
    enc1,
    enc10,
    enc11,
    enc12,
    enc2,
    enc3,
    enc4,
    enc5,
    enc6,
    enc7,
    enc8,
    enc9,
    encBottomValue,
    encTopValue,
    graphBounds
} from '../constantsValue';
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
                    {/* <KnobValue
                        audioPlugin="TrackFx"
                        param="VOLUME"
                        {...enc1}
                        color="tertiary"
                        track={track}
                    /> */}
                    <KnobValue
                        audioPlugin="MMFilter"
                        param="CUTOFF"
                        {...enc1}
                        color="secondary"
                        type="STRING"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin="MMFilter"
                        param="RESONANCE"
                        {...enc2}
                        color="secondary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="GAIN_CLIPPING"
                        {...enc3}
                        color="primary"
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
                        {...enc7}
                        color="secondary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin="Distortion"
                        param="DRIVE"
                        {...enc8}
                        color="quaternary"
                        track={track}
                    />

                    <KnobValue
                        audioPlugin="Distortion"
                        param="BASS"
                        {...enc11}
                        color="tertiary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="HIGH_FREQ_BOOST"
                        {...enc12}
                        color="tertiary"
                        track={track}
                    />

                    <GraphEncoder
                        bounds={graphBounds(bounds5)}
                        audioPlugin={synthName}
                        dataId="WAVEFORM"
                        renderValuesOnTop={false}
                        values={['WAVEFORM_TYPE', 'SHAPE', 'MACRO']}
                        track={track}
                    />
                    <Value
                        {...encTopValue(enc5)}
                        audioPlugin={synthName}
                        param="WAVEFORM_TYPE"
                        track={track}
                        barHeight={1}
                        // barColor="primary"
                        alignLeft
                        showLabelOverValue={1}
                    />
                    <Value
                        {...encTopValue(enc6)}
                        audioPlugin={synthName}
                        param="SHAPE"
                        track={track}
                        barHeight={1}
                        // barColor="primary"
                        alignLeft
                        showLabelOverValue={1}
                    />
                    <Value
                        {...encBottomValue(enc9)}
                        audioPlugin={synthName}
                        param="MACRO"
                        track={track}
                        barHeight={1}
                        // barColor="primary"
                        alignLeft
                        showLabelOverValue={1}
                    />
                    <Value
                        {...encBottomValue(enc10)}
                        audioPlugin={synthName}
                        param="PITCH"
                        track={track}
                        barHeight={1}
                        // barColor="primary"
                        alignLeft
                        showLabelOverValue={1}
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
