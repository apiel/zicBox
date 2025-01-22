import * as React from '@/libs/react';

import { View } from '@/libs/components/View';
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
    SnareTrack
} from '../constants';
import { Clips } from './Clips';
import { TextGridClips, TextGridClipsShifted } from './TextGrid';
import { Drum23Values, SampleValues } from './Values';

export type Props = {
    name: string;
};

export function ClipsView({ name }: Props) {
    return (
        <View name={name}>
            <TextGridClips />
            <TextGridClipsShifted />

            <Clips color={ColorTrack1} track={Drum23Track} group={0} title='Kick' />
            <Clips color={ColorTrack2} track={FmTrack} group={1} title='Fm' />
            <Clips color={ColorTrack3} track={SnareTrack} group={2} title='Snare' />
            <Clips color={ColorTrack4} track={HiHatTrack} group={3} title='Hihat' />
            <Clips color={ColorTrack5} track={SampleTrack} group={4} title='Sample' />

            <Drum23Values group={0} track={Drum23Track} />


            <SampleValues group={4} track={SampleTrack} />
        </View>
    );
}
