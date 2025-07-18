import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { Rect } from '@/libs/nativeComponents/Rect';
import { View } from '@/libs/nativeComponents/View';
import * as React from '@/libs/react';
import { MainKeys } from '../components/Common';
import { Title } from '../components/Title';
import { encTopLeft, MasterTrack, SelectorPosition } from '../constants';
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

            <MainKeys synthName={'Drum1'} forcePatchView viewName={name} />
            <Rect bounds={SelectorPosition} visibilityContext={[{ index: 254, value: 2, condition: 'SHOW_WHEN_NOT' }]} />
        </View>
    );
}
