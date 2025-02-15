import * as React from '@/libs/react';

import { View } from '@/libs/nativeComponents/View';
import {
    ColorTrack1,
    ColorTrack2,
    ColorTrack3,
    ColorTrack4,
    ColorTrack5,
    Drum23Track,
    FmTrack,
    HiHatTrack,
    SampleTrack,
    SnareTrack,
} from '../constants';
import { Clips } from './Clips';
import { TextGridClips, TextGridClipsShifted } from './TextGrid';
import { Drum23Values, FmValues, HiHatValues, MasterValues, SampleValues, SnareValues } from './Values';

export type Props = {
    name: string;
};

export function ClipsView({ name }: Props) {
    return (
        <View name={name}>
            <TextGridClips />
            <TextGridClipsShifted />

            <Clips color={ColorTrack1} track={Drum23Track} group={0} title="Kick" group_all={5} />
            <Clips color={ColorTrack2} track={FmTrack} group={1} title="Fm" group_all={5} />
            <Clips color={ColorTrack3} track={SnareTrack} group={2} title="Snare" group_all={5} />
            <Clips color={ColorTrack4} track={HiHatTrack} group={3} title="Hihat" group_all={5} />
            <Clips color={ColorTrack5} track={SampleTrack} group={4} title="Sample" group_all={5} />

            <Drum23Values group={0} track={Drum23Track} />
            <FmValues group={1} track={FmTrack} />
            <SnareValues group={2} track={SnareTrack} />
            <HiHatValues group={3} track={HiHatTrack} />
            <SampleValues group={4} track={SampleTrack} />

            <MasterValues group={5} track={0} />
        </View>
    );
}
