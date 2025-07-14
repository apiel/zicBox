import * as React from '@/libs/react';

import { StepEditSample } from '@/libs/nativeComponents/StepEditSample';
import { DrumLayout } from './Layout';

export type Props = {
    name: string;
    track: number;
    synthName: string;
    color: string;
};

export function FxView({ name, track, synthName, color }: Props) {
    return (
        <DrumLayout
            viewName={name}
            color={color}
            synthName={synthName}
            content={
                <>
                    <StepEditSample
                        audioPlugin="Distortion"
                        stepIndex={0}
                        bounds={[0, 0, 120, 10]}
                    />
                </>
            }
        />
    );
}
