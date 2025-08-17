import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { StringVal } from '@/libs/nativeComponents/StringVal';
import { ClipBar } from '../components/ClipBar';
import { KeysTracks } from '../components/KeysTracks';
import { Layout } from '../components/Layout';
import { ShiftLayout, shiftVisibilityContext, unshiftVisibilityContext } from '../components/ShiftLayout';
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
    return (
        <Layout
            viewName={name}
            color={color}
            synthName={synthName}
            title={title}
            content={
                <>
                    <ShiftLayout
                        track={track}
                        content={
                            <KnobValue
                                audioPlugin={synthName}
                                param="ENGINE"
                                {...enc2}
                                color="secondary"
                                track={track}
                                visibilityContext={[shiftVisibilityContext]}
                            />
                        }
                    />

                    <StringVal audioPlugin={synthName} param="ENGINE" bounds={[260, 0, 100, 20]} />

                    <KnobValue
                        audioPlugin={synthName}
                        param="VAL_7"
                        {...enc1}
                        color="secondary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <KnobValue
                        audioPlugin={synthName}
                        param="VAL_8"
                        {...enc2}
                        color="quaternary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <KnobValue
                        audioPlugin={synthName}
                        param="VAL_9"
                        {...enc3}
                        color="tertiary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <KnobValue
                        audioPlugin={synthName}
                        param="VAL_10"
                        {...enc4}
                        color="primary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <ClipBar track={track} color={color} />
                    <KeysTracks synthName={synthName} viewName={name} track={track} />
                </>
            }
        />
    );
}
