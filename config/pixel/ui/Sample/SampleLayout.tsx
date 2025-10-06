import * as React from '@/libs/react';

import { Sample } from '@/libs/nativeComponents/Sample';
import { StringVal } from '@/libs/nativeComponents/StringVal';
import { Layout } from '../components/Layout';
import {
    ShiftLayout,
    unshiftVisibilityContext
} from '../components/ShiftLayout';
import { Track } from '../components/Track';
import { Val } from '../components/Val';
import { ScreenWidth } from '../constants';
import {
    enc10smpl,
    enc11smpl,
    enc12smpl,
    enc1smpl,
    enc2smpl,
    enc3smpl,
    enc4smpl,
    enc5smpl,
    enc6smpl,
    enc7smpl,
    enc8smpl,
    enc9smpl,
    top
} from '../constantsValue';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
    title: string;
};

export function SampleLayout({ name, track, synthName, color, title }: Props) {
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
                    <ShiftLayout synthName={synthName} track={track} />

                    <StringVal
                        audioPlugin={synthName}
                        param="ENGINE"
                        bounds={[105, 3, 50, 20]}
                        fontLabel="PoppinsLight_12"
                    />

                    <Val
                        {...enc1smpl}
                        audioPlugin={synthName}
                        param="BROWSER"
                        fontValue="PoppinsLight_8"
                        track={track}
                        color={isPage1 ? 'primary' : undefined}
                    />
                    <Val
                        {...enc2smpl}
                        audioPlugin={synthName}
                        param="VAL_1"
                        track={track}
                        color={isPage1 ? 'secondary' : undefined}
                    />
                    <Val
                        {...enc3smpl}
                        audioPlugin={synthName}
                        param="VAL_2"
                        track={track}
                        color={isPage1 ? 'quaternary' : undefined}
                    />
                    <Val
                        {...enc4smpl}
                        audioPlugin={synthName}
                        param="VAL_3"
                        track={track}
                        color={isPage1 ? 'tertiary' : undefined}
                    />

                    <Val
                        {...enc5smpl}
                        audioPlugin={synthName}
                        param="VAL_4"
                        track={track}
                        color={isPage2 ? 'secondary' : undefined}
                    />
                    <Val
                        {...enc6smpl}
                        audioPlugin={synthName}
                        param="VAL_5"
                        track={track}
                        color={isPage2 ? 'quaternary' : undefined}
                    />
                    <Val
                        {...enc7smpl}
                        audioPlugin={synthName}
                        param="VAL_6"
                        track={track}
                        color={isPage2 ? 'tertiary' : undefined}
                    />
                    <Val
                        {...enc8smpl}
                        audioPlugin={synthName}
                        param="VAL_7"
                        track={track}
                        color={isPage2 ? 'primary' : undefined}
                    />

                    <Sample
                        bounds={[0, top + 35, ScreenWidth - 1, 47]}
                        audioPlugin={synthName}
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                        valueKeys={{
                            loopPosition: 'VAL_1',
                            loopLength: 'VAL_2',
                            start: 'VAL_4',
                            end: 'VAL_5',
                        }}
                    />

                    <Val
                        {...enc9smpl}
                        audioPlugin={synthName}
                        param="VAL_8"
                        track={track}
                        color={isPage3 ? 'secondary' : undefined}
                    />
                    <Val
                        {...enc10smpl}
                        audioPlugin={synthName}
                        param="VAL_9"
                        track={track}
                        color={isPage3 ? 'quaternary' : undefined}
                    />
                    <Val
                        {...enc11smpl}
                        audioPlugin={synthName}
                        param="VAL_10"
                        track={track}
                        color={isPage3 ? 'tertiary' : undefined}
                    />
                    <Val
                        {...enc12smpl}
                        audioPlugin={synthName}
                        param="VAL_11"
                        track={track}
                        color={isPage3 ? 'primary' : undefined}
                    />

                    <Track synthName={synthName} viewName={name} track={track} color={color} />
                </>
            }
        />
    );
}
