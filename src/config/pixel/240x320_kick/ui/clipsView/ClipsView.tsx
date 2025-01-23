import * as React from '@/libs/react';

import { View } from '@/libs/components/View';
import {
    ColorTrack1,
    ColorTrack2,
    Drum23Track,
    SampleTrack,
} from '../constants';
import { Clips } from './Clips';
import { TextGridClips, TextGridClipsShifted } from './TextGrid';
import { Drum23Values, MasterValues, SampleValues } from './Values';

export type Props = {
    name: string;
};

export function ClipsView({ name }: Props) {
    return (
        <View name={name}>
            <TextGridClips />
            <TextGridClipsShifted />

            <Clips color={ColorTrack1} track={Drum23Track} group={0} title="Kick" group_all={5} />
            <Clips color={ColorTrack2} track={SampleTrack} group={4} title="Sample" group_all={5} />

            <Drum23Values group={0} track={Drum23Track} />
            <SampleValues group={1} track={SampleTrack} />

            <MasterValues group={2} track={0} />
        </View>
    );
}
