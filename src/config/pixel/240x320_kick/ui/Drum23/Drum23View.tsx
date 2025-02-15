import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import { ColorTrack1, Drum23Track } from '../constants';
import { bottomLeftKnob, bottomRightKnob, topLeftKnob, topRightKnob } from '../constantsValue';
import { TextGridDrum23 } from './TextGridDrum23';

export type Props = {
    name: string;
};

export function Drum23View({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                audioPlugin="TrackFx" param="VOLUME"
                bounds={topLeftKnob}
                encoderId={0}
                color="tertiary"
                track={Drum23Track}
            />
            <KnobValue
                audioPlugin="MMFilter" param="CUTOFF"
                bounds={topRightKnob}
                encoderId={1}
                color="secondary"
                valueSize={8}
                type="STRING"
                track={Drum23Track}
            />
            <KnobValue
                audioPlugin="Drum23" param="GAIN_CLIPPING"
                bounds={bottomLeftKnob}
                encoderId={2}
                track={Drum23Track}
            />
            <KnobValue
                audioPlugin="MMFilter" param="RESONANCE"
                bounds={bottomRightKnob}
                encoderId={3}
                color="secondary"
                track={Drum23Track}
            />

            <TextGridDrum23 selected={0} viewName={name} />
            <Common selected={0} track={Drum23Track} selectedBackground={ColorTrack1} />
        </View>
    );
}
