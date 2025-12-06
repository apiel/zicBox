import * as React from '@/libs/react';

import { ResizeType } from '@/libs/nativeComponents/component';
import { GraphValue } from '@/libs/nativeComponents/GraphValue';

import { Val } from './components/Val';
import { unshiftVisibilityContext, W1_6, W2_6, W3_6 } from './constants';
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
    graphHeight
} from './constantsValue';
import { Track } from './Track';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
    title: string;
};

export function PatchEdit({ name, track, synthName, color, title }: Props) {
    const resizeType = ResizeType.RESIZE_W | ResizeType.RESIZE_X;
    return (
        <>
            <Track synthName={synthName} viewName={name} track={track} color={color} />
            <Val
                {...enc1mini}
                audioPlugin={synthName}
                param="VAL_1"
                track={track}
                color={'secondary'}
                resizeType={resizeType}
            />

            <GraphValue
                bounds={[W1_6 + 3, enc2mini.bounds[1] - 24, W1_6 - 6, graphHeight]}
                audioPlugin={synthName}
                param="VAL_2"
                outlineColor="quaternary"
                fillColor="#c2af6b"
                track={track}
                visibilityContext={[unshiftVisibilityContext]}
                resizeType={resizeType}
                extendEncoderIdArea={enc2mini.encoderId}
            />
            <Val
                {...enc2mini}
                audioPlugin={synthName}
                param="VAL_2"
                track={track}
                color={'quaternary'}
                resizeType={resizeType}
            />

            <GraphValue
                bounds={[W2_6 + 3, enc3mini.bounds[1] - 24, W1_6 - 6, graphHeight]}
                audioPlugin={synthName}
                param="VAL_3"
                outlineColor="#399462"
                fillColor="#235e3e"
                track={track}
                visibilityContext={[unshiftVisibilityContext]}
                resizeType={resizeType}
                extendEncoderIdArea={enc3mini.encoderId}
            />
            <Val
                {...enc3mini}
                audioPlugin={synthName}
                param="VAL_3"
                track={track}
                color={'tertiary'}
                resizeType={resizeType}
            />

            <GraphValue
                bounds={[W3_6 + 3, enc4mini.bounds[1] - 24, W1_6 - 6, graphHeight]}
                audioPlugin={synthName}
                param="VAL_4"
                outlineColor="primary"
                fillColor="#315c79"
                track={track}
                visibilityContext={[unshiftVisibilityContext]}
                resizeType={resizeType}
                extendEncoderIdArea={enc4mini.encoderId}
            />
            <Val
                {...enc4mini}
                audioPlugin={synthName}
                param="VAL_4"
                track={track}
                color={'primary'}
                resizeType={resizeType}
            />

            <GraphValue
                bounds={[0 + 3, enc5mini.bounds[1] - 24, W1_6 - 6, graphHeight]}
                audioPlugin={synthName}
                param="VAL_5"
                outlineColor="secondary"
                fillColor="#ad6565ff"
                track={track}
                visibilityContext={[unshiftVisibilityContext]}
                resizeType={resizeType}
                extendEncoderIdArea={enc5mini.encoderId}
            />
            <Val
                {...enc5mini}
                audioPlugin={synthName}
                param="VAL_5"
                track={track}
                // color="secondary"
                color={'secondary'}
                resizeType={resizeType}
            />

            <GraphValue
                bounds={[W1_6 + 3, enc6mini.bounds[1] - 24, W1_6 - 6, graphHeight]}
                audioPlugin={synthName}
                param="VAL_6"
                outlineColor="quaternary"
                fillColor="#c2af6b"
                track={track}
                visibilityContext={[unshiftVisibilityContext]}
                resizeType={resizeType}
                extendEncoderIdArea={enc6mini.encoderId}
            />
            <Val
                {...enc6mini}
                audioPlugin={synthName}
                param="VAL_6"
                track={track}
                color={'quaternary'}
                resizeType={resizeType}
            />

            <GraphValue
                bounds={[W2_6 + 3, enc7mini.bounds[1] - 24, W1_6 - 6, graphHeight]}
                audioPlugin={synthName}
                param="VAL_7"
                outlineColor="tertiary"
                fillColor="#235e3e"
                track={track}
                visibilityContext={[unshiftVisibilityContext]}
                resizeType={resizeType}
                extendEncoderIdArea={enc7mini.encoderId}
            />
            <Val
                {...enc7mini}
                audioPlugin={synthName}
                param="VAL_7"
                track={track}
                color={'tertiary'}
                resizeType={resizeType}
            />

            <GraphValue
                bounds={[W3_6 + 3, enc8mini.bounds[1] - 24, W1_6 - 6, graphHeight]}
                audioPlugin={synthName}
                param="VAL_8"
                outlineColor="primary"
                fillColor="#315c79"
                track={track}
                visibilityContext={[unshiftVisibilityContext]}
                resizeType={resizeType}
                extendEncoderIdArea={enc8mini.encoderId}
            />
            <Val
                {...enc8mini}
                audioPlugin={synthName}
                param="VAL_8"
                track={track}
                color={'primary'}
                resizeType={resizeType}
            />

            <GraphValue
                bounds={[0 + 3, enc9mini.bounds[1] - 24, W1_6 - 6, graphHeight]}
                audioPlugin={synthName}
                param="VAL_9"
                outlineColor="secondary"
                fillColor="#ad6565ff"
                track={track}
                visibilityContext={[unshiftVisibilityContext]}
                resizeType={resizeType}
                extendEncoderIdArea={enc9mini.encoderId}
            />
            <Val
                {...enc9mini}
                audioPlugin={synthName}
                param="VAL_9"
                track={track}
                color={'secondary'}
                resizeType={resizeType}
            />

            <GraphValue
                bounds={[W1_6 + 3, enc10mini.bounds[1] - 24, W1_6 - 6, graphHeight]}
                audioPlugin={synthName}
                param="VAL_10"
                outlineColor="quaternary"
                fillColor="#c2af6b"
                track={track}
                visibilityContext={[unshiftVisibilityContext]}
                resizeType={resizeType}
                extendEncoderIdArea={enc10mini.encoderId}
            />
            <Val
                {...enc10mini}
                audioPlugin={synthName}
                param="VAL_10"
                track={track}
                color={'quaternary'}
                resizeType={resizeType}
            />

            <GraphValue
                bounds={[W2_6 + 3, enc11mini.bounds[1] - 24, W1_6 - 6, graphHeight]}
                audioPlugin={synthName}
                param="VAL_11"
                outlineColor="tertiary"
                fillColor="#235e3e"
                track={track}
                visibilityContext={[unshiftVisibilityContext]}
                resizeType={resizeType}
                extendEncoderIdArea={enc11mini.encoderId}
            />
            <Val
                {...enc11mini}
                audioPlugin={synthName}
                param="VAL_11"
                track={track}
                color={'tertiary'}
                resizeType={resizeType}
            />

            <GraphValue
                bounds={[W3_6 + 3, enc12mini.bounds[1] - 24, W1_6 - 6, graphHeight]}
                audioPlugin={synthName}
                param="VAL_12"
                outlineColor="primary"
                fillColor="#315c79"
                track={track}
                visibilityContext={[unshiftVisibilityContext]}
                resizeType={resizeType}
                extendEncoderIdArea={enc12mini.encoderId}
            />
            <Val
                {...enc12mini}
                audioPlugin={synthName}
                param="VAL_12"
                track={track}
                color={'primary'}
                resizeType={resizeType}
            />
        </>
    );
}
