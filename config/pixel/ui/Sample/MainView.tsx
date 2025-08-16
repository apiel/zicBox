import * as React from '@/libs/react';

import { Sample } from '@/libs/nativeComponents/Sample';
import { Value } from '@/libs/nativeComponents/Value';
import { ClipSelection } from '../components/ClipSelection';
import { KeysTracks } from '../components/KeysTracks';
import { Layout } from '../components/Layout';
import { ShiftLayout, unshiftVisibilityContext } from '../components/ShiftLayout';
import {
    enc1,
    enc2,
    enc3,
    enc4,
    encTopValue,
    graphBounds
} from '../constantsValue';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
    title: string;
};

export function MainView({ name, track, synthName, color, title }: Props) {
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
                    <Value
                        {...encTopValue(enc1)}
                        audioPlugin={synthName}
                        param="START"
                        track={track}
                        barHeight={1}
                        // barColor="primary"
                        alignLeft
                        showLabelOverValue={1}
                        visibilityContext={[unshiftVisibilityContext]}
                    />
                    <Value
                        {...encTopValue(enc2)}
                        audioPlugin={synthName}
                        param="END"
                        track={track}
                        barHeight={1}
                        // barColor="primary"
                        alignLeft
                        showLabelOverValue={1}
                        visibilityContext={[unshiftVisibilityContext]}
                    />
                    <Value
                        {...encTopValue(enc3)}
                        audioPlugin="Filter"
                        param="CUTOFF"
                        track={track}
                        barHeight={1}
                        // barColor="primary"
                        alignLeft
                        showLabelOverValue={1}
                        visibilityContext={[unshiftVisibilityContext]}
                    />
                    <Value
                        {...encTopValue(enc4)}
                        audioPlugin="Filter"
                        param="RESONANCE"
                        track={track}
                        barHeight={1}
                        // barColor="primary"
                        alignLeft
                        showLabelOverValue={1}
                        visibilityContext={[unshiftVisibilityContext]}
                    />
                    <Sample
                        bounds={graphBounds}
                        audioPlugin={synthName}
                        track={track}
                        visibilityContext={[unshiftVisibilityContext]}
                    />

                    <ClipSelection track={track} color={color} />
                    <KeysTracks synthName={synthName} viewName={name} track={track} />
                </>
            }
        />
    );
}
