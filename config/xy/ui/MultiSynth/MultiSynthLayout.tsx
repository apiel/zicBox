import * as React from '@/libs/react';

import { Clip } from '@/libs/nativeComponents/Clip';
import { GraphValue } from '@/libs/nativeComponents/GraphValue';
import { Rect } from '@/libs/nativeComponents/Rect';
import { Text } from '@/libs/nativeComponents/Text';
import { View } from '@/libs/nativeComponents/View';
import { rgb } from '@/libs/ui';
import { unshiftVisibilityContext } from '../components/ShiftLayout';
import { Track } from '../components/Track';
import { Val } from '../components/Val';
import {
    A2,
    A3,
    B5,
    B6,
    B7,
    B8,
    MasterTrack,
    menuTextColor,
    ScreenHeight,
    ScreenWidth,
    shiftContext,
    shiftVisibilityContext,
    W1_8,
    W2_8,
    W4_8,
    W6_8,
} from '../constants';

const top = 0;
const rowHeight = 56;
const colNum = 4;
const margin = 4;
const width = ScreenWidth / colNum;

const bgColor = '#3a3a3a';

function getFillColor(col: number) {
    switch (col) {
        case 0:
            return '#ad6565';
        case 1:
            return '#c2af6b';
        case 2:
            return '#235e3e';
    }
    return '#315c79';
}

function getColor(col: number) {
    switch (col) {
        case 0:
            return 'secondary';
        case 1:
            return 'quaternary';
        case 2:
            return 'tertiary';
    }
    return 'primary';
}

const ValGraph =
    (track: number, synthName: string, param: string) =>
    (row: number, color: string, fillColor: string, encoderId: number, rectBounds: number[], isActive: boolean) => {
        // rectBounds[3]

        const graphBounds = [rectBounds[0] + 1, rectBounds[1], width - margin, 40];
        const bounds = [graphBounds[0], rectBounds[1] + rectBounds[3] - 28, graphBounds[2], 28];

        return (
            <>
                <Rect bounds={rectBounds} color={bgColor} extendEncoderIdArea={encoderId} visibilityContext={[unshiftVisibilityContext]} />
                <GraphValue
                    audioPlugin={synthName}
                    param={param}
                    outlineColor={isActive ? color : '#666666'}
                    fillColor={isActive ? fillColor : '#444444'}
                    track={track}
                    visibilityContext={[unshiftVisibilityContext]}
                    extendEncoderIdArea={encoderId}
                    bounds={graphBounds}
                    bgColor={bgColor}
                />
                <Val
                    encoderId={encoderId}
                    bounds={bounds}
                    audioPlugin={synthName}
                    param={param}
                    track={track}
                    color={isActive ? color : undefined}
                    bgColor={bgColor}
                />
            </>
        );
    };

const ValClip =
    (track: number, trackColor: string) =>
    (row: number, color: string, fillColor: string, encoderId: number, rectBounds: number[], isActive: boolean) => {
        return (
            <Clip
                bounds={rectBounds}
                bgColor={trackColor}
                track={track}
                smallFont="PoppinsLight_8"
                // visibilityContext={[unshiftVisibilityContext]} // Need to be always visible so action happen on shift mode
                encoderId={encoderId}
                keys={[
                    { key: A2, action: `.mute`, context: { id: shiftContext, value: 1 } },
                    { key: B5, action: `.next`, context: { id: shiftContext, value: 1 } },
                    { key: B6, action: `.load`, context: { id: shiftContext, value: 1 } },
                    { key: B7, action: `.save`, context: { id: shiftContext, value: 1 } },
                    { key: B8, action: `.delete`, context: { id: shiftContext, value: 1 } },
                ]}
            />
        );
    };

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
    title: string;
};

export function MultiSynthLayout({ name, track, synthName, color, title }: Props) {
    const isPage1 = !name.includes(':page');
    const isPage2 = name.includes(':page2');
    const isPage3 = name.includes(':page3');
    const isPage4 = name.includes(':page4');

    // Prepare all values in a grid
    const valConfigs = [
        ValClip(track, color),
        ValGraph(track, synthName, 'ENGINE'),
        ValGraph(track, synthName, 'YO2'),
        ValGraph(MasterTrack, 'Mixer', `TRACK_${track}`),

        ValGraph(track, synthName, 'VAL_1'),
        ValGraph(track, synthName, 'VAL_2'),
        ValGraph(track, synthName, 'VAL_3'),
        ValGraph(track, synthName, 'VAL_4'),

        ValGraph(track, synthName, 'VAL_5'),
        ValGraph(track, synthName, 'VAL_6'),
        ValGraph(track, synthName, 'VAL_7'),
        ValGraph(track, synthName, 'VAL_8'),

        ValGraph(track, synthName, 'VAL_9'),
        ValGraph(track, synthName, 'VAL_10'),
        ValGraph(track, synthName, 'VAL_11'),
        ValGraph(track, synthName, 'VAL_12'),
    ];

    return (
        <View name={name}>
            <Shift track={track} synthName={synthName} color={color} />

            {valConfigs.map((valGraph, index) => {
                const row = Math.floor(index / colNum);
                const col = index % colNum;
                const rectBounds = [col * width + 1, top + row * rowHeight, width - 2, rowHeight - 3];

                if (row > 0) {
                    rectBounds[1] += 18;
                    // rectBounds[1] -= 13;
                } else {
                    rectBounds[3] = 40;
                }

                const isActive = (isPage1 && row === 0) || (isPage2 && row === 1) || (isPage3 && row === 2) || (isPage4 && row === 3);

                const encoderId = isActive ? col + 1 : -1;

                return valGraph(row, getColor(col), getFillColor(col), encoderId, rectBounds, isActive);
            })}

            <Track synthName={synthName} viewName={name} track={track} color={color} />
        </View>
    );
}

function Shift({ track, synthName, color }: { track: number; synthName: string; color: string }) {
    const row1 = ['&icon::play::filled', 'Mute', 'Rec', 'Preset', 'Shift'];
    const row2 = ['Menu', '---', '---', '---', 'Next', 'Load', 'Save', '&icon::trash'];
    return (
        <>
            <Rect
                bounds={[0, ScreenHeight - 50, ScreenWidth, 50]}
                color="background"
                visibilityContext={[shiftVisibilityContext]}
                keys={[
                    {
                        key: A3,
                        action: `contextToggle:${shiftContext}:1:0`,
                        action2: `setView:${synthName}Keyboard`,
                        context: { id: shiftContext, value: 1 },
                    },
                ]}
            />
            <Rect bounds={[0, ScreenHeight - 50, ScreenWidth, 50]} color="background" visibilityContext={[unshiftVisibilityContext]} />
            <Rect bounds={[W4_8, ScreenHeight - 25, ScreenWidth - W4_8, 25]} color={color} visibilityContext={[shiftVisibilityContext]} />

            {row1.map((text, index) => {
                return (
                    <Text
                        text={text}
                        bounds={[index * W1_8, ScreenHeight - (text[0] === '&' ? 44 : 46), W1_8, 16]}
                        centered={true}
                        visibilityContext={[shiftVisibilityContext]}
                        color={text === 'Shift' ? rgb(80, 80, 80) : menuTextColor}
                    />
                );
            })}
            {row2.map((text, index) => {
                return (
                    <Text
                        text={text}
                        bounds={[index * W1_8, ScreenHeight - (text[0] === '&' ? 20 : 22), W1_8 - (text[0] === '&' ? 14 : 0), 16]}
                        centered={true}
                        visibilityContext={[shiftVisibilityContext]}
                        color={index > 3 ? 'text' : menuTextColor}
                        bgColor={index > 3 ? color : undefined}
                    />
                );
            })}

            <Text
                text="Selected clip"
                bounds={[W6_8, ScreenHeight - 33, W2_8, 10]}
                centered={true}
                visibilityContext={[shiftVisibilityContext]}
                color="text"
                bgColor={color}
                font="PoppinsLight_8"
            />
        </>
    );
}
