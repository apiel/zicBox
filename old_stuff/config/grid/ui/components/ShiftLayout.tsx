import * as React from '@/libs/react';

import { VisibilityContext } from '@/libs/nativeComponents/component';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { Rect } from '@/libs/nativeComponents/Rect';
import { Text } from '@/libs/nativeComponents/Text';
import { rgb } from '@/libs/ui';
import { shiftContext } from '../constants';
import { backgroundBounds, enc1 } from '../constantsValue';

export const unshiftVisibilityContext: VisibilityContext = {
    condition: 'SHOW_WHEN',
    index: shiftContext,
    value: 0,
};

export const shiftVisibilityContext: VisibilityContext = {
    condition: 'SHOW_WHEN',
    index: shiftContext,
    value: 1,
};

export function ShiftLayout({
    content,
    track,
    label,
    info,
}: {
    content?: any;
    track: number;
    label?: string;
    info?: string;
}) {
    return (
        <>
            <Rect
                bounds={backgroundBounds}
                color="background"
                visibilityContext={[shiftVisibilityContext]}
            />

            <KnobValue
                audioPlugin="TrackFx"
                param="VOLUME"
                label={label}
                {...enc1}
                color="primary"
                track={track}
                visibilityContext={[shiftVisibilityContext]}
            />

            {info && (
                <Text
                    text={info}
                    bounds={[0, backgroundBounds[1] + backgroundBounds[3] - 20, 480, 16]}
                    centered={true}
                    color={rgb(100, 100, 100)}
                    visibilityContext={[shiftVisibilityContext]}
                />
            )}

            {content}

            <Rect
                bounds={backgroundBounds}
                color="background"
                visibilityContext={[unshiftVisibilityContext]}
            />
        </>
    );
}
