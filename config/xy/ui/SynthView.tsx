import * as React from '@/libs/react';

import { Clip } from '@/libs/nativeComponents/Clip';
import { GraphValue } from '@/libs/nativeComponents/GraphValue';
import { HiddenValue } from '@/libs/nativeComponents/HiddenValue';
import { Pattern } from '@/libs/nativeComponents/Pattern';
import { Rect } from '@/libs/nativeComponents/Rect';
import { Sample } from '@/libs/nativeComponents/Sample';
import { Text } from '@/libs/nativeComponents/Text';
import { View } from '@/libs/nativeComponents/View';
import { WatchDataContext } from '@/libs/nativeComponents/WatchDataContext';
import { TextArray } from './components/TextArray';
import { Track } from './components/Track';
import { Val } from './components/Val';
import {
    A1,
    A2,
    A3,
    A4,
    A5,
    B1,
    B3,
    B4,
    B5,
    B6,
    B7,
    B8,
    bgColor,
    engineTypeIdContext,
    MasterTrack,
    menuTextColor,
    ScreenHeight,
    ScreenWidth,
    shiftContext,
    shiftVisibilityContext,
    shutdownContext,
    unshiftVisibilityContext,
    W1_8,
    W2_8,
    W4_8,
    W6_8
} from './constants';

const top = 0;
const rowHeight = 56;
const colNum = 4;
const width = ScreenWidth / colNum;

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
    (rectBounds: number[], row: number, color: string, fillColor: string, encoderId: number, isActive: boolean) => {
        const graphBounds = [rectBounds[0] + 1, rectBounds[1], width - 2, 22];
        const graphBgBounds = [graphBounds[0], graphBounds[1], graphBounds[2], graphBounds[3] + 2];
        const bounds = [graphBounds[0], rectBounds[1] + rectBounds[3] - 28, graphBounds[2], 28];

        const visibilityContext = [unshiftVisibilityContext];
        if (row === 1) {
            visibilityContext.push({ condition: 'SHOW_WHEN_NOT', index: engineTypeIdContext + track, value: 3 });
        }
        return (
            <>
                <Rect bounds={graphBgBounds} color={bgColor} visibilityContext={visibilityContext} />
                <GraphValue
                    audioPlugin={synthName}
                    param={param}
                    outlineColor={isActive ? color : '#666666'}
                    fillColor={isActive ? fillColor : '#444444'}
                    track={track}
                    visibilityContext={visibilityContext}
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
                    visibilityContext={[unshiftVisibilityContext]}
                />
            </>
        );
    };

const ValClip =
    (track: number, trackColor: string) =>
    (rectBounds: number[], row: number, color: string, fillColor: string, encoderId: number, isActive: boolean) => {
        return (
            <Clip
                bounds={rectBounds}
                bgColor={trackColor}
                track={track}
                smallFont="PoppinsLight_8"
                // visibilityContext={[unshiftVisibilityContext]} // Need to be always visible so action happen on shift mode
                encoderId={encoderId}
                keys={[
                    { key: B1, action: `.mute`, context: { id: shiftContext, value: 1 } },
                    { key: B5, action: `.next`, context: { id: shiftContext, value: 1 } },
                    { key: B6, action: `.load`, context: { id: shiftContext, value: 1 } },
                    { key: B7, action: `.save`, context: { id: shiftContext, value: 1 } },
                    // { key: B8, action: `.delete`, context: { id: shiftContext, value: 1 } },
                    { key: B8, action: `audioEvent:SAVE_CLIP`, action2: `.message:All saved`, context: { id: shiftContext, value: 1 } },
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

export function SynthView({ name, track, synthName, color, title }: Props) {
    const isPage1 = !name.includes(':page');
    const isPage2 = name.includes(':page2');
    const isPage3 = name.includes(':page3');
    const isPage4 = name.includes(':page4');

    // Prepare all values in a grid
    const valConfigs = [
        ValClip(track, color),
        ValGraph(track, synthName, 'ENGINE'),
        (rectBounds: number[]) => (
            <Rect
                bounds={[rectBounds[0] + 1, rectBounds[1], width - 2, rectBounds[3]]}
                color={bgColor}
                visibilityContext={[unshiftVisibilityContext]}
            />
        ),
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
            <WatchDataContext
                audioPlugin={synthName}
                track={track}
                data={[{ dataId: 'GET_ENGINE_TYPE_ID', contextIndex: engineTypeIdContext + track }]}
            />
            <Rect
                bounds={[0, 41, ScreenWidth - 1, 56]}
                // color="background"
                visibilityContext={[unshiftVisibilityContext, { condition: 'SHOW_WHEN_NOT', index: engineTypeIdContext + track, value: 3 }]}
            />

            <Pattern
                bounds={[0, 42, ScreenWidth - 1, 30]}
                audioPlugin="Sequencer"
                track={track}
                color={color}
                bgColor={bgColor}
                visibilityContext={[unshiftVisibilityContext, { condition: 'SHOW_WHEN_NOT', index: engineTypeIdContext + track, value: 3 }]}
            />

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

                return valGraph(rectBounds, row, getColor(col), getFillColor(col), encoderId, isActive);
            })}
            <Sample
                bounds={[0, 41, ScreenWidth - 1, 56]}
                audioPlugin={synthName}
                track={track}
                visibilityContext={[unshiftVisibilityContext, { condition: 'SHOW_WHEN', index: engineTypeIdContext + track, value: 3 }]}
                valueKeys={{
                    loopPosition: 'VAL_2',
                    loopLength: 'VAL_3',
                    start: 'VAL_5',
                    end: 'VAL_6',
                }}
            />
            <Track viewName={name} />
            <Keys viewName={name} synthName={synthName} />
        </View>
    );
}

function Shift({ track, synthName, color }: { track: number; synthName: string; color: string }) {
    const row1 = ['&icon::play::filled', '&icon::stop::filled', 'Rec', 'Preset', 'Shift'];
    const row2 = ['Mute', '---', '---', '&icon::shutdown', 'Next', 'Load', 'Save', '> All'];
    return (
        <>
            <Rect bounds={[0, ScreenHeight - 50, ScreenWidth, 50]} color="background" visibilityContext={[shiftVisibilityContext]} />
            <Rect bounds={[0, ScreenHeight - 50, ScreenWidth, 50]} color="background" visibilityContext={[unshiftVisibilityContext]} />
            <Rect bounds={[W4_8, ScreenHeight - 25, ScreenWidth - W4_8, 25]} color={color} visibilityContext={[shiftVisibilityContext]} />

            <TextArray texts={row1} top={ScreenHeight - 46} visibilityContext={[shiftVisibilityContext]} />
            {row2.map((text, index) => {
                return (
                    <Text
                        text={text}
                        bounds={[index * W1_8, ScreenHeight - 22, W1_8, 16]}
                        centered={true}
                        visibilityContext={[
                            shiftVisibilityContext,
                            ...(index === 2 ? [{ condition: 'SHOW_WHEN', index: shutdownContext, value: 0 }] as any : []),
                        ]}
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

function Keys({ viewName, synthName }: { synthName: string; viewName: string }) {
    const pages = (baseName: string) => {
        if (viewName === baseName) {
            return `setView:${baseName}:page2#track`;
        } else if (viewName === `${baseName}:page2`) {
            return `setView:${baseName}:page3#track`;
        } else if (viewName === `${baseName}:page3`) {
            return `setView:${baseName}:page4#track`;
        }
        return `setView:${baseName}#track`;
    };

    return (
        <>
            <Text
                text="YES"
                bounds={[W2_8, ScreenHeight - 22, W1_8, 16]}
                centered={true}
                bgColor="#ffacac"
                visibilityContext={[{ condition: 'SHOW_WHEN', index: shutdownContext, value: 1 }]}
                keys={[{ key: B3, action: 'setView:ShuttingDown', action2: `sh:halt` }]}
            />

            <HiddenValue
                keys={[
                    { key: A1, action: `noteOn:${synthName}:60`, context: { id: shiftContext, value: 0 } },
                    { key: A2, action: pages(synthName) },
                    { key: A3, action: `setView:${synthName}Seq` },
                    { key: A4, action: `setView:Master` },
                    { key: A5, action: `contextToggle:${shiftContext}:1:0` },
                ]}
                visibilityContext={[unshiftVisibilityContext]}
            />

            <HiddenValue // When shifted
                keys={[
                    { key: A1, action: `playPause` },
                    { key: A2, action: `stop` },
                    { key: A3, action: `contextToggle:${shiftContext}:1:0`, action2: `setView:${synthName}Rec` },
                    { key: A4, action: `contextToggle:${shiftContext}:1:0`, action2: `setView:${synthName}Preset` },
                    { key: A5, action: `contextToggle:${shiftContext}:1:0` },

                    // { key: B3, action: 'setView:ShuttingDown', action2: `sh:halt`, context: { id: shutdownContext, value: 1 } },
                    { key: B4, action: `contextToggle:${shutdownContext}:1:0` },
                ]}
                visibilityContext={[shiftVisibilityContext]}
            />
        </>
    );
}
