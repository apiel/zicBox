import * as React from '@/libs/react';

import { Clip } from '@/libs/nativeComponents/Clip';
import { GraphValue } from '@/libs/nativeComponents/GraphValue';
import { Rect } from '@/libs/nativeComponents/Rect';
import { View } from '@/libs/nativeComponents/View';
import { ShiftLayout, unshiftVisibilityContext } from '../components/ShiftLayout';
import { Track } from '../components/Track';
import { Val } from '../components/Val';
import { MasterTrack, ScreenWidth } from '../constants';

const top = 8;
const rowHeight = 56;
const colNum = 4;
const valHeight = 28;
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
        const graphBounds = [rectBounds[0] + 1, top + row * rowHeight, width - margin, 40];
        const bounds = [graphBounds[0], graphBounds[1] + graphBounds[3] / 2 + 4, graphBounds[2], valHeight];

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
    (track: number, trackColor: string) => (row: number, color: string, fillColor: string, encoderId: number, rectBounds: number[], isActive: boolean) => {
        return (
            <Clip
                bounds={rectBounds}
                bgColor={trackColor}
                track={track}
                smallFont="PoppinsLight_8"
                visibilityContext={[unshiftVisibilityContext]}
                encoderId={encoderId}
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
            <ShiftLayout track={track} synthName={synthName} />

            {valConfigs.map((valGraph, index) => {
                const row = Math.floor(index / colNum);
                const col = index % colNum;
                const rectBounds = [col * width + 1, top + row * rowHeight, width - 2, rowHeight - 3];

                const isActive =
                    (isPage1 && row === 0) ||
                    (isPage2 && row === 1) ||
                    (isPage3 && row === 2) ||
                    (isPage4 && row === 3);

                const encoderId = isActive ? col + 1 : -1;

                return valGraph(row, getColor(col), getFillColor(col), encoderId, rectBounds, isActive);
            })}

            <Track synthName={synthName} viewName={name} track={track} color={color} />
        </View>
    );
}
