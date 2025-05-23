import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import { encBottomLeft, encTopLeft, encTopRight } from '../constants';
import { bottomLeftKnob, topLeftKnob, topRightKnob } from '../constantsValue';
import { ViewSelector } from './ViewSelector';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function Env2View({ name, track, synthName, color }: Props) {
    return (
        <View name={name}>
            <KnobValue
                audioPlugin={synthName}
                param="CUTOFF_MOD"
                bounds={topLeftKnob}
                encoderId={encTopLeft}
                color="tertiary"
                track={track}
            />
            <KnobValue
                audioPlugin={synthName}
                param="RESONANCE_MOD"
                bounds={topRightKnob}
                encoderId={encTopRight}
                color="primary"
                track={track}
            />
            <KnobValue
                audioPlugin={synthName}
                param="FREQUENCY_MOD"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="quaternary"
                track={track}
            />
            {/* <KnobValue
                            audioPlugin={synthName} param="OSC_MOD"
                            bounds={bottomRightKnob}
                            encoderId={encBottomRight}
                            color="secondary"
                            track={track}
                        /> */}

            <ViewSelector selected={'Mod.'} viewName={name} synthName={synthName} color={color} />
            <Common track={track} selectedBackground={color} selected={synthName} synthName={synthName} />
        </View>
    );
}
