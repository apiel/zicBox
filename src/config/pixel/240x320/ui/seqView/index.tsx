import * as React from '@/libs/react';

import { HiddenValue } from '@/libs/nativeComponents/HiddenValue';
import { Keymap } from '@/libs/nativeComponents/Keymap';
import { View } from '@/libs/nativeComponents/View';
import {
    ColorTrack1,
    ColorTrack2,
    ColorTrack3,
    ColorTrack4,
    ColorTrack5,
    ColorTrack6,
    ColorTrack7,
    ColorTrack8
} from '@/pixel/240x320/ui/constants';
import { ProgressBar } from '../components/ProgressBar';
import { ValueBpm } from '../components/ValueBpm';
import { RowDrum23, RowMonoSample } from './Row';
import {
    TextGridProgressBar,
    TextGridProgressBarShifted,
    TextGridTrack,
    TextGridTrackShifted,
    TextGridTrackStepEdit,
} from './TextGrid';
import { Drum23Values } from './values/Drum23Values';
import { MasterValues } from './values/MasterValues';
import { MonoSampleValues } from './values/MonoSampleValues';

export type Props = {
    name: string;
};

export function SeqView({ name }: Props) {
    return (
        <View name={name}>
            <HiddenValue>
                <Keymap key="w" action="incGroup:-1" context="254:0" />
                <Keymap key="s" action="incGroup:+1" context="254:0" />
            </HiddenValue>

            <TextGridProgressBar />
            <TextGridProgressBarShifted />
            <TextGridTrack />
            <TextGridTrackStepEdit />
            <TextGridTrackShifted />

            <ValueBpm />

            <ProgressBar y={160} volume_plugin="MasterVolume VOLUME" group={0} />

            <RowDrum23 group={1} track={1} name_color={ColorTrack1} select_menu_context="11" />
            <RowDrum23 group={2} track={2} name_color={ColorTrack2} select_menu_context="12" />
            <RowMonoSample group={3} track={3} name_color={ColorTrack3} select_menu_context="13" />
            <RowMonoSample group={4} track={4} name_color={ColorTrack4} select_menu_context="14" />
            <RowMonoSample group={5} track={5} name_color={ColorTrack5} select_menu_context="15" />
            <RowMonoSample group={6} track={6} name_color={ColorTrack6} select_menu_context="16" />
            <RowMonoSample group={7} track={7} name_color={ColorTrack7} select_menu_context="17" />
            <RowMonoSample group={8} track={8} name_color={ColorTrack8} select_menu_context="18" />

            <MasterValues />
            <Drum23Values group={1} track={1} context={11} />
            <Drum23Values group={2} track={2} context={12} />
            <MonoSampleValues group={3} track={3} context={13} />
            <MonoSampleValues group={4} track={4} context={14} />
            <MonoSampleValues group={5} track={5} context={15} />
            <MonoSampleValues group={6} track={6} context={16} />
            <MonoSampleValues group={7} track={7} context={17} />
            <MonoSampleValues group={8} track={8} context={18} />
        </View>
    );
}
