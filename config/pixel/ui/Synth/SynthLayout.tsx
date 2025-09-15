import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { StringVal } from '@/libs/nativeComponents/StringVal';
import { ClipBar } from '../components/ClipBar';
import { KeysTracks } from '../components/KeysTracks';
import { Layout } from '../components/Layout';
import { notUnshiftVisibilityContext, ShiftLayout } from '../components/ShiftLayout';
import { Val } from '../components/Val';
import {
    enc10mini,
    enc11mini,
    enc12mini,
    enc1mini,
    enc2,
    enc2mini,
    enc3mini,
    enc4mini,
    enc5mini,
    enc6mini,
    enc7mini,
    enc8mini,
    enc9mini,
} from '../constantsValue';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
    title: string;
};

export function SynthLayout({ name, track, synthName, color, title }: Props) {
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
                    <ShiftLayout
                        synthName={synthName}
                        track={track}
                        content={
                            <KnobValue
                                audioPlugin={synthName}
                                param="ENGINE"
                                {...enc2}
                                color="secondary"
                                track={track}
                                visibilityContext={[notUnshiftVisibilityContext]}
                            />
                        }
                    />

                    <StringVal
                        audioPlugin={synthName}
                        param="ENGINE"
                        bounds={[92, 3, 60, 20]}
                        fontLabel="PoppinsLight_12"
                    />

                    <Val
                        {...enc1mini}
                        audioPlugin={synthName}
                        param="ATTACK"
                        track={track}
                        color={isPage1 ? 'secondary' : undefined}
                    />
                    <Val
                        {...enc2mini}
                        audioPlugin={synthName}
                        param="RELEASE"
                        track={track}
                        color={isPage1 ? 'quaternary' : undefined}
                    />
                    <Val
                        {...enc3mini}
                        audioPlugin={synthName}
                        param="VAL_1"
                        track={track}
                        color={isPage1 ? 'tertiary' : undefined}
                    />
                    <Val
                        {...enc4mini}
                        audioPlugin={synthName}
                        param="VAL_2"
                        track={track}
                        color={isPage1 ? 'primary' : undefined}
                    />

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

                    <ClipBar track={track} color={color} />
                    <KeysTracks synthName={synthName} viewName={name} track={track} />
                </>
            }
        />
    );
}
