import * as React from '@/libs/react';

import { HiddenValue } from '@/libs/nativeComponents/HiddenValue';
import { C1, C2, C3, C4, shiftContext } from '../constants';
import { pages } from './KeysTracks';

export const Clips = 'Clips';
export const Drum1 = 'Drum1';
export const Drum2 = 'Drum2';
export const Drum3 = 'Drum3';
export const Drum4 = 'Drum4';
export const Sample1 = 'Sample1';
export const Sample2 = 'Sample2';
// export const Sample3 = 'Sample3';
// export const Sample4 = 'Sample4';
export const Synth1 = 'Synth1';
export const Synth2 = 'Synth2';

function ifTrue<T>(cond: boolean, obj: T) {
    return cond ? [obj] : [];
}

export function MainKeys({
    synthName,
    viewName,
    forcePatchView = false,
}: {
    synthName?: string;
    viewName: string;
    forcePatchView?: boolean;
}) {
    return (
        <>
            <HiddenValue
                keys={[
                    ...ifTrue(!!synthName, {
                        key: C2,
                        action: pages(viewName, synthName!),
                    }),
                    { key: C3, action: `setView:${synthName}Clips` },
                    { key: C4, action: `contextToggle:${shiftContext}:1:0` },
                ]}
                visibilityContext={[{ index: shiftContext, value: 1, condition: 'SHOW_WHEN_NOT' }]}
            />

            <HiddenValue // When shifted
                keys={[
                    { key: C1, action: `playPause` },
                    { key: C2, action: `setView:Menu` },
                    { key: C3, action: `audioEvent:RELOAD_VARIATION` },
                    { key: C4, action: `contextToggle:${shiftContext}:1:0` },
                ]}
                visibilityContext={[{ index: shiftContext, value: 1, condition: 'SHOW_WHEN' }]}
            />
        </>
    );
}
