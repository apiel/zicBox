import * as React from '@/libs/react';

import { ResizeType } from '@/libs/nativeComponents/component';
import { GraphValue } from '@/libs/nativeComponents/GraphValue';
import { Rect } from '@/libs/nativeComponents/Rect';
import { StringVal } from '@/libs/nativeComponents/StringVal';
import { Text } from '@/libs/nativeComponents/Text';
import { View } from '@/libs/nativeComponents/View';
import { darken } from '@/libs/ui';

import { Container } from '@/libs/nativeComponents/Container';
import { ShiftLayout, unshiftVisibilityContext } from '../components/ShiftLayout';
import { TimelinePart } from '../components/TimelinePart';
import { Track } from '../components/Track';
import { Val } from '../components/Val';
import { W1_4, W2_4, W3_4 } from '../constants';
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
    moveUp,
    top,
} from '../constantsValue';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
    title: string;
};

const graphHeight = 40;
function enc(encVal: { encoderId: number; bounds: number[] }, isOriginalPos: boolean) {
    return isOriginalPos ? encVal : moveUp(encVal, 30);
}

export function MultiSynthLayout({ name, track, synthName, color, title }: Props) {
    const isPage1 = !name.includes(':page');
    const isPage2 = name.includes(':page2');
    const isPage3 = name.includes(':page3');
    const resizeType = ResizeType.RESIZE_W | ResizeType.RESIZE_X;
    const inactiveColor = darken(color, 0.5);
    const viewName = name;
    return (
        <View name={viewName}>
            <Container name="container1" position={[0, 0]}>
                {title && (
                    <Text
                        fontSize={16}
                        text={title}
                        bounds={[37, 0, 100, 16]}
                        font="PoppinsLight_8"
                    />
                )}
                {title && (
                    <>
                        <Rect
                            bounds={[5, 6, 6, 6]}
                            color={!viewName.includes(':page') ? color : inactiveColor}
                        />
                        <Rect
                            bounds={[15, 6, 6, 6]}
                            color={viewName.includes(':page2') ? color : inactiveColor}
                        />
                        <Rect
                            bounds={[25, 6, 6, 6]}
                            color={viewName.includes(':page3') ? color : inactiveColor}
                        />
                    </>
                )}
                <ShiftLayout track={track} synthName={synthName} />

                <StringVal
                    audioPlugin={synthName}
                    param="ENGINE"
                    bounds={[52, 3, 60, 20]}
                    fontLabel="PoppinsLight_12"
                    labelColor="#FFFFFF"
                    unit
                />

                <StringVal
                    audioPlugin={synthName}
                    param="ENGINE"
                    bounds={[90, 3, 60, 20]}
                    fontLabel="PoppinsLight_12"
                />

                <Val
                    {...enc(enc1mini, isPage1)}
                    audioPlugin={synthName}
                    param="VAL_1"
                    track={track}
                    color={isPage1 ? 'secondary' : undefined}
                    resizeType={resizeType}
                />
                {isPage1 && (
                    <GraphValue
                        bounds={[W1_4 + 3, top + 6, W1_4 - 6, graphHeight]}
                        audioPlugin={synthName}
                        param="VAL_2"
                        outlineColor="quaternary"
                        fillColor="#c2af6b"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                        resizeType={resizeType}
                        extendEncoderIdArea={enc2mini.encoderId}
                    />
                )}
                <Val
                    {...enc(enc2mini, isPage1)}
                    audioPlugin={synthName}
                    param="VAL_2"
                    track={track}
                    color={isPage1 ? 'quaternary' : undefined}
                    resizeType={resizeType}
                />

                {isPage1 && (
                    <GraphValue
                        bounds={[W2_4 + 3, top + 6, W1_4 - 6, graphHeight]}
                        audioPlugin={synthName}
                        param="VAL_3"
                        outlineColor="#399462"
                        fillColor="#235e3e"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                        resizeType={resizeType}
                        extendEncoderIdArea={enc3mini.encoderId}
                    />
                )}
                <Val
                    {...enc(enc3mini, isPage1)}
                    audioPlugin={synthName}
                    param="VAL_3"
                    track={track}
                    color={isPage1 ? 'tertiary' : undefined}
                    resizeType={resizeType}
                />

                {isPage1 && (
                    <GraphValue
                        bounds={[W3_4 + 3, top + 6, W1_4 - 6, graphHeight]}
                        audioPlugin={synthName}
                        param="VAL_4"
                        outlineColor="primary"
                        fillColor="#315c79"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                        resizeType={resizeType}
                        extendEncoderIdArea={enc4mini.encoderId}
                    />
                )}
                <Val
                    {...enc(enc4mini, isPage1)}
                    audioPlugin={synthName}
                    param="VAL_4"
                    track={track}
                    color={isPage1 ? 'primary' : undefined}
                    resizeType={resizeType}
                />

                {isPage2 && (
                    <GraphValue
                        bounds={[0 + 3, top + 46, W1_4 - 6, graphHeight]}
                        audioPlugin={synthName}
                        param="VAL_5"
                        outlineColor="secondary"
                        fillColor="#ad6565ff"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                        resizeType={resizeType}
                        extendEncoderIdArea={enc5mini.encoderId}
                    />
                )}
                <Val
                    {...enc(enc5mini, isPage1 || isPage2)}
                    audioPlugin={synthName}
                    param="VAL_5"
                    track={track}
                    // color="secondary"
                    color={isPage2 ? 'secondary' : undefined}
                    resizeType={resizeType}
                />
                {isPage2 && (
                    <GraphValue
                        bounds={[W1_4 + 3, top + 46, W1_4 - 6, graphHeight]}
                        audioPlugin={synthName}
                        param="VAL_6"
                        outlineColor="quaternary"
                        fillColor="#c2af6b"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                        resizeType={resizeType}
                        extendEncoderIdArea={enc6mini.encoderId}
                    />
                )}
                <Val
                    {...enc(enc6mini, isPage1 || isPage2)}
                    audioPlugin={synthName}
                    param="VAL_6"
                    track={track}
                    color={isPage2 ? 'quaternary' : undefined}
                    resizeType={resizeType}
                />
                {isPage2 && (
                    <GraphValue
                        bounds={[W2_4 + 3, top + 46, W1_4 - 6, graphHeight]}
                        audioPlugin={synthName}
                        param="VAL_7"
                        outlineColor="tertiary"
                        fillColor="#235e3e"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                        resizeType={resizeType}
                        extendEncoderIdArea={enc7mini.encoderId}
                    />
                )}
                <Val
                    {...enc(enc7mini, isPage1 || isPage2)}
                    audioPlugin={synthName}
                    param="VAL_7"
                    track={track}
                    color={isPage2 ? 'tertiary' : undefined}
                    resizeType={resizeType}
                />
                {isPage2 && (
                    <GraphValue
                        bounds={[W3_4 + 3, top + 46, W1_4 - 6, graphHeight]}
                        audioPlugin={synthName}
                        param="VAL_8"
                        outlineColor="primary"
                        fillColor="#315c79"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                        resizeType={resizeType}
                        extendEncoderIdArea={enc8mini.encoderId}
                    />
                )}
                <Val
                    {...enc(enc8mini, isPage1 || isPage2)}
                    audioPlugin={synthName}
                    param="VAL_8"
                    track={track}
                    color={isPage2 ? 'primary' : undefined}
                    resizeType={resizeType}
                />

                {isPage3 && (
                    <GraphValue
                        bounds={[0 + 3, top + 86, W1_4 - 6, graphHeight]}
                        audioPlugin={synthName}
                        param="VAL_9"
                        outlineColor="secondary"
                        fillColor="#ad6565ff"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                        resizeType={resizeType}
                        extendEncoderIdArea={enc9mini.encoderId}
                    />
                )}
                <Val
                    {...enc9mini}
                    audioPlugin={synthName}
                    param="VAL_9"
                    track={track}
                    color={isPage3 ? 'secondary' : undefined}
                    resizeType={resizeType}
                />
                {isPage3 && (
                    <GraphValue
                        bounds={[W1_4 + 3, top + 86, W1_4 - 6, graphHeight]}
                        audioPlugin={synthName}
                        param="VAL_10"
                        outlineColor="quaternary"
                        fillColor="#c2af6b"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                        resizeType={resizeType}
                        extendEncoderIdArea={enc10mini.encoderId}
                    />
                )}
                <Val
                    {...enc10mini}
                    audioPlugin={synthName}
                    param="VAL_10"
                    track={track}
                    color={isPage3 ? 'quaternary' : undefined}
                    resizeType={resizeType}
                />
                {isPage3 && (
                    <GraphValue
                        bounds={[W2_4 + 3, top + 86, W1_4 - 6, graphHeight]}
                        audioPlugin={synthName}
                        param="VAL_11"
                        outlineColor="tertiary"
                        fillColor="#235e3e"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                        resizeType={resizeType}
                        extendEncoderIdArea={enc11mini.encoderId}
                    />
                )}
                <Val
                    {...enc11mini}
                    audioPlugin={synthName}
                    param="VAL_11"
                    track={track}
                    color={isPage3 ? 'tertiary' : undefined}
                    resizeType={resizeType}
                />
                {isPage3 && (
                    <GraphValue
                        bounds={[W3_4 + 3, top + 86, W1_4 - 6, graphHeight]}
                        audioPlugin={synthName}
                        param="VAL_12"
                        outlineColor="primary"
                        fillColor="#315c79"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                        resizeType={resizeType}
                        extendEncoderIdArea={enc12mini.encoderId}
                    />
                )}
                <Val
                    {...enc12mini}
                    audioPlugin={synthName}
                    param="VAL_12"
                    track={track}
                    color={isPage3 ? 'primary' : undefined}
                    resizeType={resizeType}
                />
                <Track synthName={synthName} viewName={name} track={track} color={color} />
            </Container>
            <Container name="container2" position={[0, 180]}>
                <TimelinePart />
            </Container>
        </View>
    );
}
