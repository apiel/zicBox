import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { TracksSelector } from '../components/Common';
import { encBottomRight, encTopLeft, encTopRight } from '../constants';
import { bottomRightKnob, topLeftKnob, topRightKnob } from '../constantsValue';
import { Layout } from './Layout';
import { ViewSelector } from './ViewSelector';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function Layer2_2View({ name, track, synthName, color }: Props) {
    return (
        <Layout
            viewName={name}
            color={color}
            content={
                <>
                    <KnobValue
                        audioPlugin={synthName}
                        param="OSC2_FREQ"
                        bounds={topLeftKnob}
                        encoderId={encTopLeft}
                        color="tertiary"
                        track={track}
                    />
                    <KnobValue
                        audioPlugin={synthName}
                        param="LAYER2_CUTOFF"
                        bounds={topRightKnob}
                        encoderId={encTopRight}
                        color="secondary"
                        type="STRING"
                        track={track}
                    />
                    {/* <KnobValue
                audioPlugin={synthName}
                param="LAYER2_FILTER_TYPE"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="quaternary"
                track={track}
            /> */}
                    <KnobValue
                        audioPlugin={synthName}
                        param="LAYER2_RESONANCE"
                        bounds={bottomRightKnob}
                        encoderId={encBottomRight}
                        color="secondary"
                        track={track}
                    />

                    <ViewSelector
                        selected={'Layer2'}
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
