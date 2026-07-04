import * as React from '@/libs/react';

import { VisibilityContext } from '@/libs/nativeComponents/component';
import { GraphValue } from '@/libs/nativeComponents/GraphValue';
import { Rect } from '@/libs/nativeComponents/Rect';
import { Sample } from '@/libs/nativeComponents/Sample';
import { StringVal } from '@/libs/nativeComponents/StringVal';
import { WatchDataContext } from '@/libs/nativeComponents/WatchDataContext';
import { Layout } from '../components/Layout';
import { ShiftLayout, unshiftVisibilityContext } from '../components/ShiftLayout';
import { Track } from '../components/Track';
import { Val } from '../components/Val';
import { engineTypeIdContext, ScreenWidth, W1_4, W2_4, W3_4 } from '../constants';
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
    enc9mini
} from '../constantsValue';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
    title: string;
};

const graphTop = 21;
const graphHeight = 32;
// const graphBgColor = '#464545ff';
const graphBgColor = '#3f3f3fff';
function enc(encVal: { encoderId: number; bounds: number[] }, isOriginalPos: boolean) {
    // return isOriginalPos ? encVal : moveUp(encVal, 30);
    return encVal;
}

export function MultiSynthLayout({ name, track, synthName, color, title }: Props) {
    const isPage1 = !name.includes(':page');
    const isPage2 = name.includes(':page2');
    const isPage3 = name.includes(':page3');
    const visibilityNotSample: VisibilityContext = { condition: 'SHOW_WHEN_NOT', index: engineTypeIdContext + track, value: 3 };
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

                    <WatchDataContext
                        audioPlugin={synthName}
                        track={track}
                        data={[{ dataId: 'GET_ENGINE_TYPE_ID', contextIndex: engineTypeIdContext + track }]}
                    />
                    <Rect
                        bounds={[0, graphTop, ScreenWidth - 1, 36]}
                        // color="background"
                        visibilityContext={[unshiftVisibilityContext, visibilityNotSample]}
                        color={graphBgColor}
                    />

                    <Sample
                        bounds={[0, graphTop, ScreenWidth - 1, 36]}
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

                    {isPage1 && (
                        <>
                            <GraphValue
                                bounds={[W1_4 + 3, graphTop + 2, W1_4 - 6, graphHeight]}
                                audioPlugin={synthName}
                                param="VAL_2"
                                outlineColor="quaternary"
                                fillColor="#c2af6b"
                                track={track}
                                visibilityContext={[unshiftVisibilityContext, visibilityNotSample]}
                                bgColor={graphBgColor}
                            />
                            <GraphValue
                                bounds={[W2_4 + 3, graphTop + 2, W1_4 - 6, graphHeight]}
                                audioPlugin={synthName}
                                param="VAL_3"
                                outlineColor="#399462"
                                fillColor="#235e3e"
                                track={track}
                                visibilityContext={[unshiftVisibilityContext, visibilityNotSample]}
                                bgColor={graphBgColor}
                            />
                            <GraphValue
                                bounds={[W3_4 + 3, graphTop + 2, W1_4 - 6, graphHeight]}
                                audioPlugin={synthName}
                                param="VAL_4"
                                outlineColor="primary"
                                fillColor="#315c79"
                                track={track}
                                visibilityContext={[unshiftVisibilityContext, visibilityNotSample]}
                                bgColor={graphBgColor}
                            />
                        </>
                    )}
                    {isPage2 && (
                        <>
                            <GraphValue
                                bounds={[0 + 3, graphTop + 2, W1_4 - 6, graphHeight]}
                                audioPlugin={synthName}
                                param="VAL_5"
                                outlineColor="secondary"
                                fillColor="#ad6565ff"
                                track={track}
                                visibilityContext={[unshiftVisibilityContext, visibilityNotSample]}
                                bgColor={graphBgColor}
                            />
                            <GraphValue
                                bounds={[W1_4 + 3, graphTop + 2, W1_4 - 6, graphHeight]}
                                audioPlugin={synthName}
                                param="VAL_6"
                                outlineColor="quaternary"
                                fillColor="#c2af6b"
                                track={track}
                                visibilityContext={[unshiftVisibilityContext, visibilityNotSample]}
                                bgColor={graphBgColor}
                            />
                            <GraphValue
                                bounds={[W2_4 + 3, graphTop + 2, W1_4 - 6, graphHeight]}
                                audioPlugin={synthName}
                                param="VAL_7"
                                outlineColor="tertiary"
                                fillColor="#235e3e"
                                track={track}
                                visibilityContext={[unshiftVisibilityContext, visibilityNotSample]}
                                bgColor={graphBgColor}
                            />
                            <GraphValue
                                bounds={[W3_4 + 3, graphTop + 2, W1_4 - 6, graphHeight]}
                                audioPlugin={synthName}
                                param="VAL_8"
                                outlineColor="primary"
                                fillColor="#315c79"
                                track={track}
                                visibilityContext={[unshiftVisibilityContext, visibilityNotSample]}
                                bgColor={graphBgColor}
                            />
                        </>
                    )}
                    {isPage3 && (
                        <>
                            <GraphValue
                                bounds={[0 + 3, graphTop + 2, W1_4 - 6, graphHeight]}
                                audioPlugin={synthName}
                                param="VAL_9"
                                outlineColor="secondary"
                                fillColor="#ad6565ff"
                                track={track}
                                visibilityContext={[unshiftVisibilityContext, visibilityNotSample]}
                                bgColor={graphBgColor}
                            />
                            <GraphValue
                                bounds={[W1_4 + 3, graphTop + 2, W1_4 - 6, graphHeight]}
                                audioPlugin={synthName}
                                param="VAL_10"
                                outlineColor="quaternary"
                                fillColor="#c2af6b"
                                track={track}
                                visibilityContext={[unshiftVisibilityContext, visibilityNotSample]}
                                bgColor={graphBgColor}
                            />
                            <GraphValue
                                bounds={[W2_4 + 3, graphTop + 2, W1_4 - 6, graphHeight]}
                                audioPlugin={synthName}
                                param="VAL_11"
                                outlineColor="tertiary"
                                fillColor="#235e3e"
                                track={track}
                                visibilityContext={[unshiftVisibilityContext, visibilityNotSample]}
                                bgColor={graphBgColor}
                            />
                            <GraphValue
                                bounds={[W3_4 + 3, graphTop + 2, W1_4 - 6, graphHeight]}
                                audioPlugin={synthName}
                                param="VAL_12"
                                outlineColor="primary"
                                fillColor="#315c79"
                                track={track}
                                visibilityContext={[unshiftVisibilityContext, visibilityNotSample]}
                                bgColor={graphBgColor}
                            />
                        </>
                    )}

                    <Val
                        {...enc(enc1mini, isPage1)}
                        audioPlugin={synthName}
                        param="VAL_1"
                        track={track}
                        color={isPage1 ? 'secondary' : undefined}
                    />
                    <Val
                        {...enc(enc2mini, isPage1)}
                        audioPlugin={synthName}
                        param="VAL_2"
                        track={track}
                        color={isPage1 ? 'quaternary' : undefined}
                    />
                    <Val
                        {...enc(enc3mini, isPage1)}
                        audioPlugin={synthName}
                        param="VAL_3"
                        track={track}
                        color={isPage1 ? 'tertiary' : undefined}
                    />
                    <Val
                        {...enc(enc4mini, isPage1)}
                        audioPlugin={synthName}
                        param="VAL_4"
                        track={track}
                        color={isPage1 ? 'primary' : undefined}
                    />

                    <Val
                        {...enc(enc5mini, isPage1 || isPage2)}
                        audioPlugin={synthName}
                        param="VAL_5"
                        track={track}
                        // color="secondary"
                        color={isPage2 ? 'secondary' : undefined}
                    />
                    <Val
                        {...enc(enc6mini, isPage1 || isPage2)}
                        audioPlugin={synthName}
                        param="VAL_6"
                        track={track}
                        color={isPage2 ? 'quaternary' : undefined}
                    />
                    <Val
                        {...enc(enc7mini, isPage1 || isPage2)}
                        audioPlugin={synthName}
                        param="VAL_7"
                        track={track}
                        color={isPage2 ? 'tertiary' : undefined}
                    />
                    <Val
                        {...enc(enc8mini, isPage1 || isPage2)}
                        audioPlugin={synthName}
                        param="VAL_8"
                        track={track}
                        color={isPage2 ? 'primary' : undefined}
                    />

                    <Val {...enc9mini} audioPlugin={synthName} param="VAL_9" track={track} color={isPage3 ? 'secondary' : undefined} />
                    <Val {...enc10mini} audioPlugin={synthName} param="VAL_10" track={track} color={isPage3 ? 'quaternary' : undefined} />
                    <Val {...enc11mini} audioPlugin={synthName} param="VAL_11" track={track} color={isPage3 ? 'tertiary' : undefined} />
                    <Val {...enc12mini} audioPlugin={synthName} param="VAL_12" track={track} color={isPage3 ? 'primary' : undefined} />

                    <Track synthName={synthName} viewName={name} track={track} color={color} />
                </>
            }
        />
    );
}
