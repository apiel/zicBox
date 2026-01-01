import * as React from '@/libs/react';

import { VisibilityContext } from '@/libs/nativeComponents/component';
import { Value } from '@/libs/nativeComponents/Value';
import { Bounds } from '@/libs/ui';
import { unshiftVisibilityContext } from '../components/ShiftLayout';

export type Props = {
    track: number;
    audioPlugin: string;
    param: string;
    color?: string;
    bounds: Bounds;
    encoderId?: number;
    fontValue?: string;
    label?: string;
    bgColor?: string;
    visibilityContext?: VisibilityContext[];
};

export function Val({ track, param, audioPlugin, color, bounds, encoderId, fontValue, label, bgColor, visibilityContext }: Props) {
    return (
        <Value
            bounds={bounds}
            encoderId={encoderId}
            audioPlugin={audioPlugin}
            param={param}
            track={track}
            barHeight={2}
            barColor={color}
            valueColor={!color ? '#5a5a5a' : undefined}
            alignLeft
            showLabelOverValue={2}
            visibilityContext={visibilityContext || [unshiftVisibilityContext]}
            fontValue={fontValue}
            label={label}
            bgColor={bgColor}
        />
    );
}
