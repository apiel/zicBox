import * as React from '@/libs/react';

import { GraphEncoder } from '@/libs/nativeComponents/GraphEncoder';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { Value } from '@/libs/nativeComponents/Value';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import {
    encBottomLeft,
    encBottomRight,
    encTopLeft,
    encTopRight,
    ScreenWidth
} from '../constants';
import { graphBottomValues, topLeftKnob, topRightKnob } from '../constantsValue';
import { ViewSelector } from './ViewSelector';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function MainView({ name, track, synthName, color }: Props) {
    return (
        <View name={name}>
            <KnobValue
                audioPlugin="TrackFx"
                param="VOLUME"
                bounds={topLeftKnob}
                encoderId={encTopLeft}
                color="tertiary"
                track={track}
            />
            <KnobValue
                audioPlugin={synthName}
                param="PITCH"
                bounds={topRightKnob}
                encoderId={encTopRight}
                color="primary"
                track={track}
            />
            {/* <KnobValue
                audioPlugin={synthName}
                param="WAVE"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="quaternary"
                track={track}
            />
            <KnobValue
                audioPlugin={synthName}
                param="WAVE_EDIT"
                bounds={bottomRightKnob}
                encoderId={encBottomRight}
                color="secondary"
                track={track}
            /> */}

            <GraphEncoder
                bounds={graphBottomValues}
                audioPlugin={synthName}
                dataId="WAVEFORM"
                renderValuesOnTop={false}
                values={['WAVE', 'WAVE_EDIT']}
                track={track}
            />
            <Value
                bounds={[0, 240, ScreenWidth / 2 - 2, 22]}
                audioPlugin={synthName}
                param="WAVE"
                track={track}
                fontLabel="PoppinsLight_6"
                barHeight={1}
                encoderId={encBottomLeft}
                alignLeft
                showLabelOverValue={0}
            />

            <Value
                bounds={[ScreenWidth / 2 + 2, 240, ScreenWidth / 2, 22]}
                audioPlugin={synthName}
                param="WAVE_EDIT"
                track={track}
                fontLabel="PoppinsLight_6"
                barHeight={1}
                encoderId={encBottomRight}
                alignLeft
                showLabelOverValue={0}
            />

            <ViewSelector selected={'Main'} viewName={name} synthName={synthName} color={color} />
            <Common track={track} selectedBackground={color} selected={synthName} />
        </View>
    );
}
