import * as React from '@/libs/react';

import { Sample } from '@/libs/nativeComponents/Sample';
import { Value } from '@/libs/nativeComponents/Value';
import { TracksSelector } from '../components/Common';
import { Layout } from '../components/Layout';
import { encBottomLeft, encBottomRight, encTopLeft, encTopRight, ScreenWidth } from '../constants';
import { graphCenterValues } from '../constantsValue';
import { ViewSelector } from './ViewSelector';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function WaveView({ name, track, synthName, color }: Props) {
    return (
        <Layout
            viewName={name}
            color={color}
            synthName={synthName}
            title={synthName}
            content={
                <>
                    <Sample bounds={graphCenterValues} audioPlugin={synthName} track={track} />

                    <Value
                        bounds={[0, 90, ScreenWidth / 2 - 2, 22]}
                        audioPlugin={synthName}
                        param="START"
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
                        param="END"
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
                        param="LOOP_POSITION"
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
                        param="LOOP_LENGTH"
                        track={track}
                        fontLabel="PoppinsLight_6"
                        barHeight={1}
                        encoderId={encBottomRight}
                        alignLeft
                        showLabelOverValue={0}
                    />

                    <ViewSelector
                        selected={'Wave'}
                        viewName={name}
                        synthName={synthName}
                        color={color}
                    />
                    <TracksSelector
                        selectedBackground={color}
                        viewName={name}
                        trackName={synthName}
                    />
                </>
            }
        />
    );
}
