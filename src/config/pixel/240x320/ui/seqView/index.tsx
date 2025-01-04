import * as React from '@/libs/react';

import { HiddenValue } from '@/libs/components/HiddenValue';
import { Keymap } from '@/libs/components/Keymap';
import { Spectrogram } from '@/libs/components/Spectrogram';
import { View } from '@/libs/components/View';
import {
    ColorTrack1,
    ColorTrack2,
    ColorTrack3,
    ColorTrack4,
    ColorTrack5,
    ColorTrack6,
    ColorTrack7,
    ColorTrack8,
    ScreenWidth,
} from '@/pixel/240x320/ui/constants';
import { ProgressBar } from '../components/ProgressBar';
import { ValueBpm } from '../components/ValueBpm';
import { RowDrum23, RowDrumSample } from './Row';
import {
    TextGridProgressBar,
    TextGridProgressBarShifted,
    TextGridTrack,
    TextGridTrackShifted,
    TextGridTrackStepEdit,
} from './TextGrid';
import { DrumSampleValues } from './values/DrumSampleValues';
import { MasterValues } from './values/MasterValues';

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

            <Spectrogram
                position={[0, 4, ScreenWidth, 70]}
                track={0}
                data="Spectrogram BUFFER"
                text="Pixel"
            />

            <ProgressBar y={160} volume_plugin="MasterVolume VOLUME" group={0} />

            <RowDrum23 group={1} track={1} name_color={ColorTrack1} select_menu_context="11" />
            <RowDrum23 group={2} track={2} name_color={ColorTrack2} select_menu_context="12" />
            <RowDrumSample group={3} track={3} name_color={ColorTrack3} select_menu_context="13" />
            <RowDrumSample group={4} track={4} name_color={ColorTrack4} select_menu_context="14" />
            <RowDrumSample group={5} track={5} name_color={ColorTrack5} select_menu_context="15" />
            <RowDrumSample group={6} track={6} name_color={ColorTrack6} select_menu_context="16" />
            <RowDrumSample group={7} track={7} name_color={ColorTrack7} select_menu_context="17" />
            <RowDrumSample group={8} track={8} name_color={ColorTrack8} select_menu_context="18" />

            <MasterValues />


            <DrumSampleValues group={3} track={3} context={13} />
            <DrumSampleValues group={4} track={4} context={14} />
            <DrumSampleValues group={5} track={5} context={15} />
            <DrumSampleValues group={6} track={6} context={16} />
            <DrumSampleValues group={7} track={7} context={17} />
            <DrumSampleValues group={8} track={8} context={18} />
        </View>
    );
}
