import * as React from '@/libs/react';

import { GraphEncoder } from '@/libs/nativeComponents/GraphEncoder';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { StringVal } from '@/libs/nativeComponents/StringVal';
import { Value } from '@/libs/nativeComponents/Value';
import { rgb } from '@/libs/ui';
import { ClipSelection } from '../components/ClipSelection';
import { KeysTracks } from '../components/KeysTracks';
import { Layout } from '../components/Layout';
import { ShiftLayout, shiftVisibilityContext, unshiftVisibilityContext } from '../components/ShiftLayout';
import {
    bounds2,
    boundsMarginTop,
    enc1,
    enc2,
    enc3,
    enc4,
    encBottomValue,
} from '../constantsValue';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
    title: string;
};

export function MainView({ name, track, synthName, color, title }: Props) {
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
                        param="DURATION"
                        {...enc1}
                        color="secondary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <GraphEncoder
                        bounds={boundsMarginTop(bounds2)}
                        audioPlugin={synthName}
                        dataId="ENV_AMP_FORM"
                        values={['AMP_MORPH']}
                        outlineColor="quaternary"
                        fillColor={rgb(194, 175, 107)}
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />
                    <Value
                        {...encBottomValue(enc2)}
                        audioPlugin={synthName}
                        param="AMP_MORPH"
                        track={track}
                        barHeight={1}
                        barColor="quaternary"
                        alignLeft
                        showLabelOverValue={1}
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <KnobValue
                        audioPlugin={synthName}
                        param="VAL_1"
                        {...enc3}
                        color="tertiary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <KnobValue
                        audioPlugin={synthName}
                        param="VAL_2"
                        {...enc4}
                        color="primary"
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <ClipSelection track={track} color={color} />
                    <KeysTracks synthName={synthName} viewName={name} />
                </>
            }
        />
    );
}
