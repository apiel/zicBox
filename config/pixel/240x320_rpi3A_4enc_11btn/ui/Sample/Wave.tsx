import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { Sample } from '@/libs/nativeComponents/Sample';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import {
    encTopRight
} from '../constants';
import { graphCenterValues, topRightKnob } from '../constantsValue';
import { ViewSelector } from './ViewSelector';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function WaveView({ name, track, synthName, color }: Props) {
    return (
        <View name={name}>
            <KnobValue
                audioPlugin={synthName}
                param="PITCH"
                bounds={topRightKnob}
                encoderId={encTopRight}
                color="tertiary"
                track={track}
            />
            <Sample
                bounds={graphCenterValues}
                audioPlugin={synthName}
                track={track}
            />
            {/* <Value
                bounds={[0, 240, ScreenWidth / 2 - 2, 22]}
                audioPlugin={synthName}
                param="WAVE"
                track={track}
                fontLabel="PoppinsLight_6"
                barHeight={1}
                encoderId={encBottomLeft}
                alignLeft
                showLabelOverValue={0}
            /> */}

            {/* <Value
                bounds={[ScreenWidth / 2 + 2, 240, ScreenWidth / 2, 22]}
                audioPlugin={synthName}
                param="WAVE_EDIT"
                track={track}
                fontLabel="PoppinsLight_6"
                barHeight={1}
                encoderId={encBottomRight}
                alignLeft
                showLabelOverValue={0}
            /> */}

            <ViewSelector selected={'Wave'} viewName={name} synthName={synthName} color={color} />
            <Common track={track} selectedBackground={color} selected={synthName} />
        </View>
    );
}
