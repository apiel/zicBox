import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { ClipBar } from '../components/ClipBar';
import { KeysTracks } from '../components/KeysTracks';
import { Layout } from '../components/Layout';
import { ShiftLayout, unshiftVisibilityContext } from '../components/ShiftLayout';
import {
    enc1,
    enc2,
    enc3,
    enc4
} from '../constantsValue';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
    title: string;
};

export function Main3View({ name, track, synthName, color, title }: Props) {
    // const enc3Bounds = encTopValue(enc3);
    // enc3Bounds.bounds[2] = W1_2 - 2;
    return (
        <Layout
            viewName={name}
            color={color}
            synthName={synthName}
            title={title}
            content={
                <>
                    <ShiftLayout track={track} />
                
                    <KnobValue
                        audioPlugin="TrackFx"
                        param="FX_TYPE"
                        {...enc1}
                        color="secondary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />
                    <KnobValue
                        audioPlugin="TrackFx"
                        param="FX_AMOUNT"
                        {...enc2}
                        color="secondary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />
                    <KnobValue
                        audioPlugin="TrackFx2"
                        param="FX_TYPE"
                        label="FX2 type"
                        {...enc3}
                        color="quaternary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />
                    <KnobValue
                        audioPlugin="TrackFx2"
                        param="FX_AMOUNT"
                        label="FX2 edit"
                        {...enc4}
                        color="quaternary"
                        track={track}
                        valueReplaceTitle
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <ClipBar track={track} color={color} />
                    <KeysTracks synthName={synthName} viewName={name} track={track} />
                </>
            }
        />
    );
}
