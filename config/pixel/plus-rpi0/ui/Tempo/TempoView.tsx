import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
import * as React from '@/libs/react';
import { Title } from '../components/Title';
import { encTopLeft, MasterTrack } from '../constants';
import { topLeftKnob } from '../constantsValue';

export type Props = {
    name: string;
};

export function TempoView({ name }: Props) {
    return (
        <View name={name}>
            <Title title="Tempo" />
            <KnobValue
                audioPlugin="Tempo"
                param="BPM"
                bounds={topLeftKnob}
                encoderId={encTopLeft}
                color="tertiary"
                track={MasterTrack}
            />
            {/* <KnobValue
                audioPlugin="Filter"
                param="CUTOFF"
                bounds={topRightKnob}
                encoderId={encTopRight}
                type="STRING"
                color="primary"
                track={track}
            /> */}
            {/* <KnobValue
                audioPlugin="Filter"
                param="CUTOFF"
                type="STRING"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="primary"
                track={track}
            /> */}
            {/* <KnobValue
                audioPlugin="Filter"
                param="RESONANCE"
                bounds={bottomRightKnob}
                encoderId={encBottomRight}
                color="primary"
                track={track}
            /> */}

            {/* <ShiftedTextGrid selected={name} />
            <TracksTextGrid selected={name} contextValue={0} /> */}
        </View>
    );
}
