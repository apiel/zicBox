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
            <Clips color={ColorTrack1} track={Drum23Track} group={0} title="Kick" group_all={8} />
            <Clips color={ColorTrack2} track={Synth1Track} group={1} title="Synth1" group_all={8} />
            <Clips color={ColorTrack3} track={Synth2Track} group={2} title="Synth2" group_all={8} />
            <Clips color={ColorTrack4} track={Synth3Track} group={3} title="Synth3" group_all={8} />
            <Clips color={ColorTrack5} track={PercTrack} group={4} title="Perc" group_all={8} />
            <Clips color={ColorTrack6} track={Sample1Track} group={5} title="Sample1" group_all={8} />
            <Clips color={ColorTrack7} track={Sample2Track} group={6} title="Sample2" group_all={8} />
            <Clips color={ColorTrack8} track={Sample3Track} group={7} title="Sample3" group_all={8} />

            {/* <Drum23Values group={0} track={Drum23Track} /> */}
            {/* <SampleValues group={1} track={SampleTrack} /> */}

            {/* <MasterValues group={2} track={0} /> */}

            <TextGridClips />
            <ShiftedTextGrid selectedBackground={ColorTrackMaster} selected="Clips" />
        </View>
    );
}
