import * as React from '@/libs/react';

import { HiddenContext } from '@/libs/nativeComponents/HiddenContext';
import { View } from '@/libs/nativeComponents/View';
import { ShiftedTextGrid } from '../components/ShiftedTextGrid';
import {
    btn1,
    btn2,
    btn3,
    btn4,
    btn5,
    btn6,
    btn7,
    btn8,
    ColorTrack1,
    ColorTrack2,
    ColorTrack3,
    ColorTrack4,
    ColorTrack5,
    ColorTrack6,
    ColorTrack7,
    ColorTrack8,
    ColorTrackMaster,
    Drum1Track,
    KickTrack,
    Sample1Track,
    Sample2Track,
    Sample3Track,
    Synth1Track,
    Synth2Track,
    Synth3Track
} from '../constants';
import { Clips } from './Clips';
import { TextGridClips } from './TextGrid';

export type Props = {
    name: string;
};

export function ClipsView({ name }: Props) {
    return (
        <View name={name}>
            {/* Put ShiftedTextGrid first, so it will overtake clip key actions */}
            <ShiftedTextGrid selectedBackground={ColorTrackMaster} selected="Clips" />

            <Clips color={ColorTrack1} track={KickTrack} title="T.1" key={btn1} />
            <Clips color={ColorTrack2} track={Synth1Track} title="T.2" key={btn2} />
            <Clips color={ColorTrack3} track={Synth2Track} title="T.3" key={btn3} />
            <Clips color={ColorTrack4} track={Synth3Track} title="T.4" key={btn4} />
            <Clips color={ColorTrack5} track={Drum1Track} title="T.5" key={btn5} />
            <Clips color={ColorTrack6} track={Sample1Track} title="T.6" key={btn6} />
            <Clips color={ColorTrack7} track={Sample2Track} title="T.7" key={btn7} />
            <Clips color={ColorTrack8} track={Sample3Track} title="T.8" key={btn8} />
            <HiddenContext encoderId={0} contextId={30} min={0} max={16} inverted />

            <TextGridClips />
        </View>
    );
}
