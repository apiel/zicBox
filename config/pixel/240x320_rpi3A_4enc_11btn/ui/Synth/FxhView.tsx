import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { View } from '@/libs/nativeComponents/View';
import { Common } from '../components/Common';
import { ColorTrack4, encBottomLeft, encTopLeft, encTopRight, SynthTrack } from '../constants';
import { bottomLeftKnob, topLeftKnob, topRightKnob } from '../constantsValue';
import { TextGridSynth } from './TextGridSynth';

export type Props = {
    name: string;
};

export function SynthFxView({ name }: Props) {
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
                audioPlugin="TrackFx"
                param="FX_TYPE"
                bounds={topRightKnob}
                encoderId={encTopRight}
                color="primary"
                track={SynthTrack}
                valueReplaceTitle
            />
            <KnobValue
                audioPlugin="TrackFx"
                param="FX_AMOUNT"
                bounds={bottomLeftKnob}
                encoderId={encBottomLeft}
                color="quaternary"
                track={SynthTrack}
            />
            {/* <KnobValue
                audioPlugin="Synth" param="MOD_INDEX"
                bounds={bottomRightKnob}
                encoderId={encBottomRight}
                color="secondary"
                track={SynthTrack}
            /> */}

            <TextGridSynth selected={'Fx'} viewName={name} />
            <Common selected={'Synth'} track={SynthTrack} selectedBackground={ColorTrack4} />
        </View>
    );
}
