import * as React from '@/libs/react';

import { HiddenValue } from '@/libs/nativeComponents/HiddenValue';
import {
    A1,
    B9,
    shiftContext
} from './constants';

export function Track({
    synthName,
    viewName,
    track,
    color,
}: {
    synthName: string;
    viewName: string;
    track: number;
    color: string;
}) {
    return (
        <>
            <HiddenValue
                keys={[
                    { key: A1, action: `noteOn:${synthName}:60` },
                    { key: B9, action: `contextToggle:${shiftContext}:1:0` },
                ]}
                visibilityContext={[{ index: shiftContext, value: 1, condition: 'SHOW_WHEN_NOT' }]}
            />

            <HiddenValue // When shifted
                keys={[
                    // { key: A1, action: `playPause` },
                    { key: A1, action: `playStop` },
                    { key: B9, action: `contextToggle:${shiftContext}:1:0` },
                ]}
                visibilityContext={[{ index: shiftContext, value: 1, condition: 'SHOW_WHEN' }]}
            />
        </>
    );
}
