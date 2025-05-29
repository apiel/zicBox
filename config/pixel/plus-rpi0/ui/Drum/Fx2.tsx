import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { TracksSelector } from '../components/Common';
import { encTopLeft, encTopRight } from '../constants';
import { topLeftKnob, topRightKnob } from '../constantsValue';
import { Layout } from './Layout';
import { ViewSelector } from './ViewSelector';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function Fx2View({ name, track, synthName, color }: Props) {
    return (
        <Layout
            viewName={name}
            color={color}
            content={
                <>
                    <KnobValue
                        audioPlugin={synthName}
                        param="HIGH_FREQ_BOOST"
                        bounds={topLeftKnob}
                        encoderId={encTopLeft}
                        color="tertiary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="GAIN_CLIPPING"
                        bounds={topRightKnob}
                        encoderId={encTopRight}
                        color="primary"
                        track={track}
                    />
                    {/* <KnobValue
                audioPlugin="Distortion" param="DRIVE"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="quaternary"
                track={track}
            /> */}
                    {/* <KnobValue
                audioPlugin="Distortion" param="BASS"
                bounds={bottomRightKnob}
                encoderId={encBottomRight}
                color="secondary"
                track={track}
            /> */}
                    <ViewSelector
                        selected={'Fx'}
                        color={color}
                        synthName={synthName}
                        viewName={name}
                        pageCount={2}
                        currentPage={2}
                    />
                    <TracksSelector selectedBackground={color} viewName={name} />
                </>
            }
        />
    );
}
