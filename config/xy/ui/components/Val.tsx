import * as React from '@/libs/react';

import { Value } from '@/libs/nativeComponents/Value';
import { Bounds } from '@/libs/ui';
import { unshiftVisibilityContext } from '../constants';

export type Props = {
    track: number;
    audioPlugin: string;
    param: string;
    color?: string;
    bounds: Bounds;
    encoderId?: number;
    fontValue?: string;
    label?: string;
    resizeType?: number
    bgColor?: string
};

export function Val({ track, param, audioPlugin, color, bounds, encoderId, fontValue, label, resizeType, bgColor }: Props) {
    return (
        <Value
            bounds={bounds}
            encoderId={color ? encoderId : -1}
            audioPlugin={audioPlugin}
            param={param}
            track={track}
            barHeight={2}
            barColor={color}
            valueColor={!color ? '#5a5a5a' : undefined}
            alignLeft
            showLabelOverValue={2}
            visibilityContext={[unshiftVisibilityContext]}
            fontValue={fontValue}
            label={label}
            resizeType={resizeType}
            bgColor={bgColor}
        />
    );
}
