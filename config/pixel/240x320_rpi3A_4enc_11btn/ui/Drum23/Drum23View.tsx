import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { Rect } from '@/libs/nativeComponents/Rect';
import { Text } from '@/libs/nativeComponents/Text';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import {
    ColorTrack1,
    Drum23Track,
    encBottomLeft,
    encBottomRight,
    encTopLeft,
    encTopRight,
} from '../constants';
import { bottomLeftKnob, bottomRightKnob, topLeftKnob, topRightKnob } from '../constantsValue';
import { TextGridDrum23 } from './TextGridDrum23';

export type Props = {
    name: string;
};

export function Drum23View({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                audioPlugin="TrackFx"
                param="VOLUME"
                bounds={topLeftKnob}
                encoderId={encTopLeft}
                color="tertiary"
                track={Drum23Track}
            />
            <KnobValue
                audioPlugin="MMFilter"
                param="CUTOFF"
                bounds={topRightKnob}
                encoderId={encTopRight}
                color="secondary"
                valueSize={8}
                type="STRING"
                track={Drum23Track}
            />
            <KnobValue
                audioPlugin="Drum23"
                param="GAIN_CLIPPING"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                track={Drum23Track}
            />
            <KnobValue
                audioPlugin="MMFilter"
                param="RESONANCE"
                bounds={bottomRightKnob}
                encoderId={encBottomRight}
                color="secondary"
                track={Drum23Track}
            />

            <Text
                // right
                text={'HELLO " 1234 hello'}
                bounds={[10, 245, 220, 20]}
                fontSize={16}
                font="draw/ST7789/fonts/Roboto-Thin.ttf"
            />
            <Rect bounds={[10, 245, 220, 20]} color="white" filled={false} />

            {/* <Text
                // right
                text="HELLO 1234 hello"
                bounds={[10, 270, 220, 20]}
                fontSize={16}
                font="RobotoThin_16"
            />
            <Rect bounds={[10, 270, 220, 20]} color="white" filled={false} /> */}
            <Text
                // right
                text="HELLO 1234 hello"
                bounds={[10, 270, 100, 20]}
                fontSize={16}
                font="RobotoThin_16"
            />
            <Rect bounds={[10, 270, 100, 20]} color="white" filled={false} />

            <TextGridDrum23 selected={'Main'} viewName={name} />
            <Common selected={'Kick'} track={Drum23Track} selectedBackground={ColorTrack1} />
        </View>
    );
}
