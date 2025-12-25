import * as React from '@/libs/react';

import { GraphValue } from '@/libs/nativeComponents/GraphValue';
import { StringVal } from '@/libs/nativeComponents/StringVal';
import { Layout } from '../components/Layout';
import { ShiftLayout, unshiftVisibilityContext } from '../components/ShiftLayout';
import { Track } from '../components/Track';
import { Val } from '../components/Val';
import {
    enc10mini,
    enc11mini,
    enc12mini,
    enc1mini,
    enc2mini,
    enc3mini,
    enc4mini,
    enc5mini,
    enc6mini,
    enc7mini,
    enc8mini,
    enc9mini,
    moveUp
} from '../constantsValue';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
    title: string;
};

const graphHeight = 40;
function getEnc(encVal: { encoderId: number; bounds: number[] }, isOriginalPos: boolean) {
    return isOriginalPos ? encVal : moveUp(encVal, 30);
}

function ValGraph({
    track,
    synthName,
    color,
    fillColor,
    up,
    isActive,
    param,
    enc,
}: {
    track: number;
    synthName: string;
    color: string;
    fillColor: string;
    up: boolean;
    isActive: boolean;
    param: string;
    enc: { encoderId: number; bounds: number[] };
}) {
    if (!up) {
        enc = {...moveUp(enc, 30)};
    }
    return (
        <>
            {isActive && (
                <GraphValue
                    audioPlugin={synthName}
                    param={param}
                    outlineColor={color}
                    fillColor={fillColor}
                    track={track}
                    visibilityContext={[unshiftVisibilityContext]}
                    extendEncoderIdArea={enc.encoderId}
                    bounds={[enc.bounds[0], enc.bounds[1] - 24, enc.bounds[2], graphHeight]}
                />
            )}
            <Val {...enc} audioPlugin={synthName} param={param} track={track} color={isActive ? color : undefined} />
        </>
    );
}

export function MultiSynthLayout({ name, track, synthName, color, title }: Props) {
    const isPage1 = !name.includes(':page');
    const isPage2 = name.includes(':page2');
    const isPage3 = name.includes(':page3');
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

                    <Val
                        {...getEnc(enc1mini, isPage1)}
                        audioPlugin={synthName}
                        param="VAL_1"
                        track={track}
                        color={isPage1 ? 'secondary' : undefined}
                    />

                    <ValGraph
                        track={track}
                        synthName={synthName}
                        color="quaternary"
                        fillColor="#c2af6b"
                        up={isPage1}
                        isActive={isPage1}
                        enc={enc2mini}
                        param="VAL_2"
                    />
                    <ValGraph
                        track={track}
                        synthName={synthName}
                        color="tertiary"
                        fillColor="#235e3e"
                        up={isPage1}
                        isActive={isPage1}
                        enc={enc3mini}
                        param="VAL_3"
                    />
                    <ValGraph
                        track={track}
                        synthName={synthName}
                        color="primary"
                        fillColor="#315c79"
                        up={isPage1}
                        isActive={isPage1}
                        enc={enc4mini}
                        param="VAL_4"
                    />

                    <ValGraph
                        track={track}
                        synthName={synthName}
                        color="secondary"
                        fillColor="#ad6565ff"
                        up={isPage1 || isPage2}
                        isActive={isPage2}
                        enc={enc5mini}
                        param="VAL_5"
                    />
                    <ValGraph
                        track={track}
                        synthName={synthName}
                        color="quaternary"
                        fillColor="#c2af6b"
                        up={isPage1 || isPage2}
                        isActive={isPage2}
                        enc={enc6mini}
                        param="VAL_6"
                    />
                    <ValGraph
                        track={track}
                        synthName={synthName}
                        color="tertiary"
                        fillColor="#235e3e"
                        up={isPage1 || isPage2}
                        isActive={isPage2}
                        enc={enc7mini}
                        param="VAL_7"
                    />
                    <ValGraph
                        track={track}
                        synthName={synthName}
                        color="primary"
                        fillColor="#315c79"
                        up={isPage1 || isPage2}
                        isActive={isPage2}
                        enc={enc8mini}
                        param="VAL_8"
                    />

                    <ValGraph
                        track={track}
                        synthName={synthName}
                        color="secondary"
                        fillColor="#ad6565ff"
                        up
                        isActive={isPage3}
                        enc={enc9mini}
                        param="VAL_9"
                    />
                    <ValGraph
                        track={track}
                        synthName={synthName}
                        color="quaternary"
                        fillColor="#c2af6b"
                        up
                        isActive={isPage3}
                        enc={enc10mini}
                        param="VAL_10"
                    />
                    <ValGraph
                        track={track}
                        synthName={synthName}
                        color="tertiary"
                        fillColor="#235e3e"
                        up
                        isActive={isPage3}
                        enc={enc11mini}
                        param="VAL_11"
                    />
                    <ValGraph
                        track={track}
                        synthName={synthName}
                        color="primary"
                        fillColor="#315c79"
                        up
                        isActive={isPage3}
                        enc={enc12mini}
                        param="VAL_12"
                    />

                    <Track synthName={synthName} viewName={name} track={track} color={color} />
                </>
            }
        />
    );
}
