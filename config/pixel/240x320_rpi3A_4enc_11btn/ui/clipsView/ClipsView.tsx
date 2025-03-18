import * as React from '@/libs/react';

import { View } from '@/libs/nativeComponents/View';
import { ShiftedTextGrid } from '../components/ShiftedTextGrid';
import {
    ColorTrack1,
    ColorTrackMaster,
    Drum23Track
} from '../constants';
import { Clips } from './Clips';
import { TextGridClips } from './TextGrid';
import { Drum23Values, MasterValues } from './Values';

export type Props = {
    name: string;
};

export function ClipsView({ name }: Props) {
    return (
        <View name={name}>
            <Clips color={ColorTrack1} track={Drum23Track} group={0} title="Kick" group_all={2} />
            {/* <Clips color={ColorTrack2} track={BassTrack} group={1} title="Bass" group_all={2} /> */}

            <Drum23Values group={0} track={Drum23Track} />
            {/* <SampleValues group={1} track={SampleTrack} /> */}

            <MasterValues group={2} track={0} />

            <TextGridClips />
            <ShiftedTextGrid selectedBackground={ColorTrackMaster} selected="Clips" />
        </View>
    );
}
