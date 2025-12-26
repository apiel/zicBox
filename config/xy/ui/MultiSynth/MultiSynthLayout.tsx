import * as React from '@/libs/react';

import { GraphValue } from '@/libs/nativeComponents/GraphValue';
import { StringVal } from '@/libs/nativeComponents/StringVal';
import { Layout } from '../components/Layout';
import { ShiftLayout, unshiftVisibilityContext } from '../components/ShiftLayout';
import { Track } from '../components/Track';
import { Val } from '../components/Val';
import { ScreenWidth } from '../constants';

const top = 40;
const rowHeight = 60;
const colNum = 4;
const valHeight = 30;
const margin = 4;
const width = ScreenWidth / colNum - margin;

export type ValGraphProps = {
    track: number;
    synthName: string;
    color: string;
    fillColor: string;
    isActive: boolean;
    param: string;
    row: number;
    col: number;
};

function ValGraph({ track, synthName, color, fillColor, isActive, param, row, col }: ValGraphProps) {
    const encoderId = (col % colNum) + 1;

    const graphBounds = [col * (width + margin), top + row * rowHeight, width, 40];
    const bounds = [graphBounds[0], graphBounds[1] + graphBounds[3] / 2 + 4, graphBounds[2], valHeight];

    return (
        <>
            <GraphValue
                audioPlugin={synthName}
                param={param}
                outlineColor={color}
                fillColor={fillColor}
                track={track}
                visibilityContext={[unshiftVisibilityContext]} // always visible now
                extendEncoderIdArea={encoderId}
                bounds={graphBounds}
            />
            <Val
                encoderId={encoderId}
                bounds={bounds}
                audioPlugin={synthName}
                param={param}
                track={track}
                color={isActive ? color : undefined}
            />
        </>
    );
}

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

    // Prepare all values in a grid
    const valConfigs = [
        { param: 'VAL_1', pageActive: isPage1, color: 'secondary', fillColor: '#ad6565' },
        { param: 'VAL_2', pageActive: isPage1, color: 'quaternary', fillColor: '#c2af6b' },
        { param: 'VAL_3', pageActive: isPage1, color: 'tertiary', fillColor: '#235e3e' },
        { param: 'VAL_4', pageActive: isPage1, color: 'primary', fillColor: '#315c79' },

        { param: 'VAL_5', pageActive: isPage2, color: 'secondary', fillColor: '#ad6565' },
        { param: 'VAL_6', pageActive: isPage2, color: 'quaternary', fillColor: '#c2af6b' },
        { param: 'VAL_7', pageActive: isPage2, color: 'tertiary', fillColor: '#235e3e' },
        { param: 'VAL_8', pageActive: isPage2, color: 'primary', fillColor: '#315c79' },

        { param: 'VAL_9', pageActive: isPage3, color: 'secondary', fillColor: '#ad6565' },
        { param: 'VAL_10', pageActive: isPage3, color: 'quaternary', fillColor: '#c2af6b' },
        { param: 'VAL_11', pageActive: isPage3, color: 'tertiary', fillColor: '#235e3e' },
        { param: 'VAL_12', pageActive: isPage3, color: 'primary', fillColor: '#315c79' },
    ];

    return (
        <Layout
            viewName={name}
            color={color}
            synthName={synthName}
            title={title}
            content={
                <>
                    <ShiftLayout track={track} synthName={synthName} />

                    <StringVal
                        audioPlugin={synthName}
                        param="ENGINE"
                        bounds={[52, 3, 60, 20]}
                        fontLabel="PoppinsLight_12"
                        labelColor="#FFFFFF"
                        unit
                    />

                    <StringVal audioPlugin={synthName} param="ENGINE" bounds={[90, 3, 60, 20]} fontLabel="PoppinsLight_12" />

                    {valConfigs.map((v, index) => {
                        const row = Math.floor(index / colNum);
                        const col = index % colNum;
                        return (
                            <ValGraph
                                track={track}
                                synthName={synthName}
                                color={v.color}
                                fillColor={v.fillColor}
                                isActive={v.pageActive}
                                param={v.param}
                                row={row}
                                col={col}
                            />
                        );
                    })}

                    <Track synthName={synthName} viewName={name} track={track} color={color} />
                </>
            }
        />
    );
}
