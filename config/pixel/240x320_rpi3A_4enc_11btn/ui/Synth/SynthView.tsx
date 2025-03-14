import * as React from '@/libs/react';

import { GraphEncoder } from '@/libs/nativeComponents/GraphEncoder';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { Value } from '@/libs/nativeComponents/Value';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import { ColorTrack4, encBottomLeft, encBottomRight, encTopLeft, encTopRight, ScreenWidth, SynthTrack } from '../constants';
import { graphBottomValues, topLeftKnob, topRightKnob } from '../constantsValue';
import { TextGridSynth } from './TextGridSynth';

export type Props = {
    name: string;
};

export function SynthView({ name }: Props) {
    return (
        <View name={name}>
            <KnobValue
                audioPlugin="TrackFx"
                param="VOLUME"
                bounds={topLeftKnob}
                encoderId={encTopLeft}
                color="tertiary"
                track={SynthTrack}
            />
            <KnobValue
                audioPlugin="Synth"
                param="PITCH"
                bounds={topRightKnob}
                encoderId={encTopRight}
                color="primary"
                track={SynthTrack}
            />
            {/* <KnobValue
                audioPlugin="Synth"
                param="WAVE"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="quaternary"
                track={SynthTrack}
            />
            <KnobValue
                audioPlugin="Synth"
                param="WAVE_EDIT"
                bounds={bottomRightKnob}
                encoderId={encBottomRight}
                color="secondary"
                track={SynthTrack}
            /> */}

            <GraphEncoder
                bounds={graphBottomValues}
                audioPlugin="Synth"
                dataId="WAVEFORM"
                renderValuesOnTop={false}
                values={['WAVE', 'WAVE_EDIT']}
                track={SynthTrack}
            />
            <Value
                bounds={[0, 240, ScreenWidth / 2 - 2, 22]}
                audioPlugin="Synth"
                param="WAVE"
                track={SynthTrack}
                fontLabel="PoppinsLight_6"
                barHeight={1}
                encoderId={encBottomLeft}
                alignLeft
                showLabelOverValue={0}
            />

            <Value
                bounds={[ScreenWidth / 2 + 2, 240, ScreenWidth / 2, 22]}
                audioPlugin="Synth"
                param="WAVE_EDIT"
                track={SynthTrack}
                fontLabel="PoppinsLight_6"
                barHeight={1}
                encoderId={encBottomRight}
                alignLeft
                showLabelOverValue={0}
            />

            <TextGridSynth selected={'Main'} viewName={name} />
            <Common selected={'Synth'} track={SynthTrack} selectedBackground={ColorTrack4} />
        </View>
    );
}
