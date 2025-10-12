import * as React from '@/libs/react';

import { GraphEncoder } from '@/libs/nativeComponents/GraphEncoder';
import { GraphValue } from '@/libs/nativeComponents/GraphValue';
import { Rect } from '@/libs/nativeComponents/Rect';
import { StringVal } from '@/libs/nativeComponents/StringVal';
import { rgb } from '@/libs/ui';
import { Layout } from '../components/Layout';
import { ShiftLayout, unshiftVisibilityContext } from '../components/ShiftLayout';
import { Track } from '../components/Track';
import { Val } from '../components/Val';
import { ScreenWidth, W1_4, W2_4, W3_4 } from '../constants';
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
    top,
} from '../constantsValue';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
    title: string;
};

export function DrumLayout({ name, track, synthName, color, title }: Props) {
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
                        bounds={[90, 3, 60, 20]}
                        fontLabel="PoppinsLight_12"
                    />

                    <Val
                        {...enc1mini}
                        audioPlugin={synthName}
                        param="DURATION"
                        track={track}
                        color={isPage1 ? 'secondary' : undefined}
                    />
                    <GraphEncoder
                        bounds={[W1_4 + 3, top + 5, W1_4 - 6, 40]}
                        audioPlugin={synthName}
                        dataId="ENV_AMP_FORM"
                        values={['AMP_MORPH']}
                        outlineColor={isPage1 ? 'quaternary' : rgb(90, 90, 90)}
                        fillColor={isPage1 ? rgb(194, 175, 107) : rgb(90, 90, 90)}
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                        inverted
                    />
                    <Val
                        {...enc2mini}
                        audioPlugin={synthName}
                        param="AMP_MORPH"
                        track={track}
                        color={isPage1 ? 'quaternary' : undefined}
                    />

                    {/* <GraphEncoder
                        bounds={[W2_4 + 3, top + 5, W1_4 - 6, 40]}
                        audioPlugin={synthName}
                        dataId="VAL_1_GRAPH"
                        values={['VAL_1']}
                        outlineColor={isPage1 ? rgb(57, 148, 98) : rgb(90, 90, 90)}
                        fillColor={isPage1 ? rgb(35, 94, 62) : rgb(90, 90, 90)}
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                        inverted
                    /> */}
                    <GraphValue
                        bounds={[W2_4 + 3, top + 5, W1_4 - 6, 40]}
                        audioPlugin={synthName}
                        param="VAL_1"
                        outlineColor={isPage1 ? rgb(57, 148, 98) : rgb(90, 90, 90)}
                        fillColor={isPage1 ? rgb(35, 94, 62) : rgb(90, 90, 90)}
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />
                    <Val
                        {...enc3mini}
                        audioPlugin={synthName}
                        param="VAL_1"
                        track={track}
                        color={isPage1 ? 'tertiary' : undefined}
                    />

                    {/* <GraphEncoder
                        bounds={[W3_4 + 3, top + 5, W1_4 - 6, 40]}
                        audioPlugin={synthName}
                        dataId="VAL_2_GRAPH"
                        values={['VAL_2']}
                        outlineColor={isPage1 ? 'primary' : rgb(90, 90, 90)}
                        fillColor={isPage1 ? rgb(49, 92, 121) : rgb(90, 90, 90)}
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                        inverted
                    /> */}
                    <GraphValue
                        bounds={[W3_4 + 3, top + 5, W1_4 - 6, 40]}
                        audioPlugin={synthName}
                        param="VAL_2"
                        outlineColor={isPage1 ? 'primary' : rgb(90, 90, 90)}
                        fillColor={isPage1 ? rgb(49, 92, 121) : rgb(90, 90, 90)}
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />
                    <Val
                        {...enc4mini}
                        audioPlugin={synthName}
                        param="VAL_2"
                        track={track}
                        color={isPage1 ? 'primary' : undefined}
                    />

                    {/* <Rect bounds={[W3_4 + 3, top, W1_4 - 6, 30]} color="#000" /> */}
                    <Rect bounds={[ScreenWidth - 2, top, 2, 30]} color="#FFFFFF" />

                    <Val
                        {...enc5mini}
                        audioPlugin={synthName}
                        param="VAL_3"
                        track={track}
                        // color="secondary"
                        color={isPage2 ? 'secondary' : undefined}
                    />
                    <Val
                        {...enc6mini}
                        audioPlugin={synthName}
                        param="VAL_4"
                        track={track}
                        color={isPage2 ? 'quaternary' : undefined}
                    />
                    <Val
                        {...enc7mini}
                        audioPlugin={synthName}
                        param="VAL_5"
                        track={track}
                        color={isPage2 ? 'tertiary' : undefined}
                    />
                    <Val
                        {...enc8mini}
                        audioPlugin={synthName}
                        param="VAL_6"
                        track={track}
                        color={isPage2 ? 'primary' : undefined}
                    />

                    <Val
                        {...enc9mini}
                        audioPlugin={synthName}
                        param="VAL_7"
                        track={track}
                        color={isPage3 ? 'secondary' : undefined}
                    />
                    <Val
                        {...enc10mini}
                        audioPlugin={synthName}
                        param="VAL_8"
                        track={track}
                        color={isPage3 ? 'quaternary' : undefined}
                    />
                    <Val
                        {...enc11mini}
                        audioPlugin={synthName}
                        param="VAL_9"
                        track={track}
                        color={isPage3 ? 'tertiary' : undefined}
                    />
                    <Val
                        {...enc12mini}
                        audioPlugin={synthName}
                        param="VAL_10"
                        track={track}
                        color={isPage3 ? 'primary' : undefined}
                    />

                    <Track synthName={synthName} viewName={name} track={track} color={color} />
                </>
            }
        />
    );
}
