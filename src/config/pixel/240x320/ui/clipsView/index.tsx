import * as React from '@/libs/react';

import { View } from '@/libs/components/View';
import { ProgressBar } from '../components/ProgressBar';
import { ValueBpm } from '../components/ValueBpm';
import {
    ColorTrack1,
    ColorTrack2,
    ColorTrack3,
    ColorTrack4,
    ColorTrack5,
    ColorTrack6,
    ColorTrack7,
    ColorTrack8,
} from '../constants';
import { Clips } from './Clips';

export type Props = {
    name: string;
};

export function ClipsView({ name }: Props) {
    return (
        <View name={name}>
            <ProgressBar y={5} />
            <ValueBpm />

            <Clips color={ColorTrack1} track={1} group={0} />
            <Clips color={ColorTrack2} track={2} group={1} />
            <Clips color={ColorTrack3} track={3} group={2} />
            <Clips color={ColorTrack4} track={4} group={3} />
            <Clips color={ColorTrack5} track={5} group={3} />
            <Clips color={ColorTrack6} track={6} group={4} />
            <Clips color={ColorTrack7} track={7} group={4} />
            <Clips color={ColorTrack8} track={8} group={5} />
        </View>
    );
}
