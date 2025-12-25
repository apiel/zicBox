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

                    {/* {isPage2 && (
                        <GraphValue
                            bounds={[0 + 3, top + 46, W1_4 - 6, graphHeight]}
                            audioPlugin={synthName}
                            param="VAL_5"
                            outlineColor="secondary"
                            fillColor="#ad6565ff"
                            track={track}
                            visibilityContext={[unshiftVisibilityContext]}
                        />
                    )}
                    <Val
                        {...getEnc(enc5mini, isPage1 || isPage2)}
                        audioPlugin={synthName}
                        param="VAL_5"
                        track={track}
                        // color="secondary"
                        color={isPage2 ? 'secondary' : undefined}
                    /> */}
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

                    {/* {isPage2 && (
                        <GraphValue
                            bounds={[W1_4 + 3, top + 46, W1_4 - 6, graphHeight]}
                            audioPlugin={synthName}
                            param="VAL_6"
                            outlineColor="quaternary"
                            fillColor="#c2af6b"
                            track={track}
                            visibilityContext={[unshiftVisibilityContext]}
                        />
                    )}
                    <Val
                        {...getEnc(enc6mini, isPage1 || isPage2)}
                        audioPlugin={synthName}
                        param="VAL_6"
                        track={track}
                        color={isPage2 ? 'quaternary' : undefined}
                    /> */}
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

                    {/* {isPage2 && (
                        <GraphValue
                            bounds={[W2_4 + 3, top + 46, W1_4 - 6, graphHeight]}
                            audioPlugin={synthName}
                            param="VAL_7"
                            outlineColor="tertiary"
                            fillColor="#235e3e"
                            track={track}
                            visibilityContext={[unshiftVisibilityContext]}
                        />
                    )}
                    <Val
                        {...getEnc(enc7mini, isPage1 || isPage2)}
                        audioPlugin={synthName}
                        param="VAL_7"
                        track={track}
                        color={isPage2 ? 'tertiary' : undefined}
                    /> */}

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

                    {/* {isPage2 && (
                        <GraphValue
                            bounds={[W3_4 + 3, top + 46, W1_4 - 6, graphHeight]}
                            audioPlugin={synthName}
                            param="VAL_8"
                            outlineColor="primary"
                            fillColor="#315c79"
                            track={track}
                            visibilityContext={[unshiftVisibilityContext]}
                        />
                    )}
                    <Val
                        {...getEnc(enc8mini, isPage1 || isPage2)}
                        audioPlugin={synthName}
                        param="VAL_8"
                        track={track}
                        color={isPage2 ? 'primary' : undefined}
                    /> */}

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

                    {/* {isPage3 && (
                        <GraphValue
                            bounds={[0 + 3, top + 86, W1_4 - 6, graphHeight]}
                            audioPlugin={synthName}
                            param="VAL_9"
                            outlineColor="secondary"
                            fillColor="#ad6565ff"
                            track={track}
                            visibilityContext={[unshiftVisibilityContext]}
                        />
                    )}
                    <Val {...enc9mini} audioPlugin={synthName} param="VAL_9" track={track} color={isPage3 ? 'secondary' : undefined} /> */}

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

                    {/* {isPage3 && (
                        <GraphValue
                            bounds={[W1_4 + 3, top + 86, W1_4 - 6, graphHeight]}
                            audioPlugin={synthName}
                            param="VAL_10"
                            outlineColor="quaternary"
                            fillColor="#c2af6b"
                            track={track}
                            visibilityContext={[unshiftVisibilityContext]}
                        />
                    )}
                    <Val {...enc10mini} audioPlugin={synthName} param="VAL_10" track={track} color={isPage3 ? 'quaternary' : undefined} /> */}

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

                    {/* {isPage3 && (

                        <GraphValue
                            bounds={[W2_4 + 3, top + 86, W1_4 - 6, graphHeight]}
                            audioPlugin={synthName}
                            param="VAL_11"
                            outlineColor="tertiary"
                            fillColor="#235e3e"
                            track={track}
                            visibilityContext={[unshiftVisibilityContext]}
                        />
                    )}
                    <Val {...enc11mini} audioPlugin={synthName} param="VAL_11" track={track} color={isPage3 ? 'tertiary' : undefined} /> */}

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


                    {/* {isPage3 && (
                        <GraphValue
                            bounds={[W3_4 + 3, top + 86, W1_4 - 6, graphHeight]}
                            audioPlugin={synthName}
                            param="VAL_12"
                            outlineColor="primary"
                            fillColor="#315c79"
                            track={track}
                            visibilityContext={[unshiftVisibilityContext]}
                        />
                    )}
                    <Val {...enc12mini} audioPlugin={synthName} param="VAL_12" track={track} color={isPage3 ? 'primary' : undefined} /> */}

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
