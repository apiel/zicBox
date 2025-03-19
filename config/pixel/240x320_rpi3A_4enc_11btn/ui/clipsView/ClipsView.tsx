import * as React from '@/libs/react';

import { View } from '@/libs/nativeComponents/View';
import { ShiftedTextGrid } from '../components/ShiftedTextGrid';
import {
    ColorTrack1,
    ColorTrack2,
    ColorTrack3,
    ColorTrack4,
    ColorTrack5,
    ColorTrack6,
    ColorTrack7,
    ColorTrack8,
    ColorTrackMaster,
    Drum23Track,
    PercTrack,
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
            <Clips color={ColorTrack1} track={Drum23Track} title="T.1" />
            <Clips color={ColorTrack2} track={Synth1Track} title="T.2" />
            <Clips color={ColorTrack3} track={Synth2Track} title="T.3" />
            <Clips color={ColorTrack4} track={Synth3Track} title="T.4" />
            <Clips color={ColorTrack5} track={PercTrack} title="T.5" />
            <Clips color={ColorTrack6} track={Sample1Track} title="T.6" />
            <Clips color={ColorTrack7} track={Sample2Track} title="T.7" />
            <Clips color={ColorTrack8} track={Sample3Track} title="T.8" />

            <TextGridClips />
            <ShiftedTextGrid selectedBackground={ColorTrackMaster} selected="Clips" />
        </View>
    );
}
