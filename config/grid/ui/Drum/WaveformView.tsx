import * as React from '@/libs/react';

import { GraphEncoder } from '@/libs/nativeComponents/GraphEncoder';
import { Value } from '@/libs/nativeComponents/Value';
import { Drum1, TracksSelector } from '../components/Common';
import { encBottomLeft, encBottomRight, encTopLeft, encTopRight, ScreenWidth } from '../constants';
import { graphCenterValues } from '../constantsValue';
import { DrumLayout } from './Layout';
import { ViewSelector } from './ViewSelector';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function WaveformView({ name, track, synthName, color }: Props) {
    return (
        <DrumLayout
            viewName={name}
            color={color}
            synthName={synthName}
            content={
                <>
                    <GraphEncoder
                        bounds={graphCenterValues}
                        audioPlugin={synthName}
                        dataId="WAVEFORM"
                        renderValuesOnTop={false}
                        values={['WAVEFORM_TYPE', 'SHAPE', 'MACRO']}
                        track={track}
                    />

                    <Value
                        bounds={[0, 90, ScreenWidth / 2 - 2, 22]}
                        audioPlugin={synthName}
                        param="WAVEFORM_TYPE"
                        track={track}
                        fontLabel="PoppinsLight_6"
                        barHeight={1}
                        encoderId={encTopLeft}
                        alignLeft
                        showLabelOverValue={0}
                    />

                    <Value
                        bounds={[ScreenWidth / 2 + 2, 90, ScreenWidth / 2, 22]}
                        audioPlugin={synthName}
                        param="SHAPE"
                        track={track}
                        fontLabel="PoppinsLight_6"
                        barHeight={1}
                        encoderId={encTopRight}
                        alignLeft
                        showLabelOverValue={0}
                    />

                    <Value
                        bounds={[0, 210, ScreenWidth / 2 - 2, 22]}
                        audioPlugin={synthName}
                        param="MACRO"
                        track={track}
                        fontLabel="PoppinsLight_6"
                        barHeight={1}
                        encoderId={encBottomLeft}
                        alignLeft
                        showLabelOverValue={0}
                    />

                    <Value
                        bounds={[ScreenWidth / 2 + 2, 210, ScreenWidth / 2, 22]}
                        audioPlugin={synthName}
                        param="PITCH"
                        track={track}
                        fontLabel="PoppinsLight_6"
                        barHeight={1}
                        encoderId={encBottomRight}
                        alignLeft
                        showLabelOverValue={0}
                    />

                    <ViewSelector
                        selected={'Wave'}
                        color={color}
                        synthName={synthName}
                        viewName={name}
                    />
                    <TracksSelector selectedBackground={color} viewName={name} trackName={Drum1} />
                </>
            }
        />
    );
}
