import * as React from '@/libs/react';

import { HiddenValue } from '@/libs/nativeComponents/HiddenValue';
import { C1, C2, C3, C4, shiftContext } from '../constants';

export const Clips = 'Clips';
export const Drum1 = 'Drum1';
export const Drum2 = 'Drum2';
export const Drum3 = 'Drum3';
export const Drum4 = 'Drum4';
export const Sample1 = 'Sample1';
export const Sample2 = 'Sample2';
export const Sample3 = 'Sample3';
export const Sample4 = 'Sample4';

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
                        action:
                            viewName === `${synthName}Seq`
                                ? `setView:${synthName}`
                                : `setView:${synthName}Seq`,
                    }),
                    {
                        key: C3,
                        action:
                            viewName === `${synthName}Clips`
                                ? `setView:${synthName}`
                                : `setView:${synthName}Clips`,
                    },
                    { key: C4, action: `contextToggle:${shiftContext}:1:0` },
                ]}
                visibilityContext={[{ index: shiftContext, value: 1, condition: 'SHOW_WHEN_NOT' }]}
            />

            <HiddenValue // When shifted
                keys={[
                    // A11 free
                    // B11 is used on track view for variation.
                    // C11 free
                    // { key: D11, action: `setView:Workspaces` },

                    { key: C1, action: `playPause` },
                    { key: C2, action: `setView:Menu` },
                    { key: C4, action: `contextToggle:${shiftContext}:1:0` },
                    // { key: C12, action: `stop` },
                    // { key: D12, action: `setView:Shutdown`,  action2: `contextToggle:${shiftContext}:1:0`  }, // action2: `setContext:${shiftContext}:0`
                ]}
                visibilityContext={[{ index: shiftContext, value: 1, condition: 'SHOW_WHEN' }]}
            />
        </>
    );
}
