import * as React from '@/libs/react';

import { HiddenValue } from '@/libs/nativeComponents/HiddenValue';
import { Keymap } from '@/libs/nativeComponents/Keymap';
import { Rect } from '@/libs/nativeComponents/Rect';
import { Value } from '@/libs/nativeComponents/Value';
import { VisibilityContainer } from '@/libs/nativeComponents/VisibilityContainer';
import {
    PrimaryBar,
    QuaternaryBar,
    ScreenWidth,
    SecondaryBar,
    TertiaryBar,
    W2_4,
} from '../constants';

const valueH = 29;
const rowSpacing = 8;

const topLeft = [0, 0, W2_4 - 2, valueH];
const bottomLeft = [0, valueH + rowSpacing, W2_4 - 2, valueH];
const topRight = [W2_4, 0, W2_4 - 2, valueH];
const bottomRight = [W2_4, valueH + rowSpacing, W2_4 - 2, valueH];

const posContainer = [0, 222, ScreenWidth, valueH * 2 + rowSpacing];

const base = {
    SHOW_LABEL_OVER_VALUE: 6,
    LABEL_FONT_SIZE: 8,
};

export const primary = {
    LABEL_COLOR: 'primary',
    BAR_COLOR: PrimaryBar,
    ...base,
};

export const secondary = {
    LABEL_COLOR: 'secondary',
    BAR_COLOR: SecondaryBar,
    ...base,
};

export const tertiary = {
    LABEL_COLOR: 'tertiary',
    BAR_COLOR: TertiaryBar,
    ...base,
};

export const quaternary = {
    LABEL_COLOR: 'quaternary',
    BAR_COLOR: QuaternaryBar,
    ...base,
};

export function MasterValues({ group, track }: { group: number; track: number }) {
    return (
        <VisibilityContainer bounds={posContainer} group={group}>
            <HiddenValue>
                <Keymap key="s" action="setView:Drum23" context="254:1" />
            </HiddenValue>
            <Rect bounds={[0, 0, ScreenWidth, posContainer[3]]} />
            <Value
                value="Volume VOLUME"
                bounds={topLeft}
                group={group}
                track={track}
                encoder_id={0}
                {...tertiary}
            />
            {/* <Value
                value="Volume GAIN_CLIPPING"
                bounds={bottomLeft}
                group={group}
                track={track}
                encoder_id={1}
                {...primary}
            /> */}
            {/* <Value
                value="MMFilter CUTOFF"
                bounds={topRight}
                group={group}
                track={track}
                encoder_id={2}
                {...quaternary}
                USE_STRING_VALUE
            />
            <Value
                value="MMFilter RESONANCE"
                bounds={bottomRight}
                group={group}
                track={track}
                encoder_id={3}
                {...quaternary}
            /> */}
        </VisibilityContainer>
    );
}

export function Drum23Values({ group, track }: { group: number; track: number }) {
    return (
        <VisibilityContainer bounds={posContainer} group={group}>
            <HiddenValue>
                <Keymap key="s" action="setView:Drum23" context="254:1" />
            </HiddenValue>
            <Value
                value="TrackFx VOLUME"
                bounds={topLeft}
                group={group}
                track={track}
                encoder_id={0}
                {...tertiary}
            />
            {/* <Value
                value="Drum23 GAIN_CLIPPING"
                bounds={bottomLeft}
                group={group}
                track={track}
                encoder_id={1}
                {...primary}
            /> */}
            <Value
                value="MMFilter CUTOFF"
                bounds={topRight}
                group={group}
                track={track}
                encoder_id={2}
                {...quaternary}
                USE_STRING_VALUE
            />
            <Value
                value="MMFilter RESONANCE"
                bounds={bottomRight}
                group={group}
                track={track}
                encoder_id={3}
                {...quaternary}
            />
        </VisibilityContainer>
    );
}

export function SampleValues({ group, track }: { group: number; track: number }) {
    return (
        <VisibilityContainer bounds={posContainer} group={group}>
            <HiddenValue>
                <Keymap key="s" action="setView:Sample" context="254:1" />
            </HiddenValue>
            <Value
                value="TrackFx VOLUME"
                bounds={topLeft}
                group={group}
                track={track}
                encoder_id={0}
                {...tertiary}
            />
            {/* <Value
                value="TrackFx REVERB"
                bounds={bottomLeft}
                group={group}
                track={track}
                encoder_id={1}
                {...primary}
            /> */}
            {/* <Value
                value="Sample START"
                bounds={topRight}
                group={group}
                track={track}
                encoder_id={2}
                {...quaternary}
                USE_STRING_VALUE
            />
            <Value
                value="Sample LOOP_LENGTH"
                bounds={bottomRight}
                group={group}
                track={track}
                encoder_id={3}
                {...quaternary}
            /> */}
            {/* <Value
                value="TrackFx CUTOFF"
                bounds={topRight}
                group={group}
                track={track}
                encoder_id={2}
                {...quaternary}
                USE_STRING_VALUE
            /> */}
            {/* <Value
                value="TrackFx RESONANCE"
                bounds={bottomRight}
                group={group}
                track={track}
                encoder_id={3}
                {...quaternary}
            /> */}
        </VisibilityContainer>
    );
}
