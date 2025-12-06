import * as React from '@/libs/react';

import { HiddenValue } from '@/libs/nativeComponents/HiddenValue';
import {
    A1,
    B9,
    C2,
    C3,
    C4,
    shiftContext,
    Track4
} from '../constants';

export function pages(viewName: string, baseName: string) {
    if (viewName === baseName) {
        return `setView:${baseName}:page2#track`;
    } else if (viewName === `${baseName}:page2`) {
        return `setView:${baseName}:page3#track`;
    }
    return `setView:${baseName}#track`;
}

function getKeys(key: string, track: number, viewName: string, baseName: string) {
    return viewName.startsWith(baseName)
        ? [
              // { key, action: pages(viewName, `Track1`), context: { id: shiftContext, value: 0 } },
              {
                  key,
                  action: `contextToggle:${shiftContext}:2:0`,
                  context: { id: shiftContext, value: 2 },
              },
              {
                  key,
                  action: `contextToggle:${shiftContext}:2:0`,
                  context: { id: shiftContext, value: 0 },
              },
              { key, action: `mute:${track}`, context: { id: shiftContext, value: 1 } },
          ]
        : [
              {
                  key,
                  action: `setView:${baseName}#track`,
                  action2: `setActiveTrack:${track}`,
                  context: { id: shiftContext, value: 0 },
              },
              { key, action: `mute:${track}`, context: { id: shiftContext, value: 1 } },
          ];
}

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
                    {
                        key: A1,
                        action:
                            track <= Track4
                                ? `noteOnAndRepeat:${synthName}:60:val:${track}`
                                : `noteOn:${synthName}:60`,
                        context: { id: shiftContext, value: 0 },
                    },
                ]}
            />

            <HiddenValue
                keys={[
                    { key: C2, action: pages(viewName, synthName!) },
                    { key: C3, action: `setView:${synthName}Clips` },
                    { key: C4, action: `contextToggle:${shiftContext}:1:0` },
                    { key: B9, action: `contextToggle:${shiftContext}:1:0` },
                ]}
                visibilityContext={[{ index: shiftContext, value: 1, condition: 'SHOW_WHEN_NOT' }]}
            />

            <HiddenValue // When shifted
                keys={[
                    { key: A1, action: `playPause` },
                    {
                        key: C2,
                        action: `contextToggle:${shiftContext}:1:0`,
                        action2: `setView:Menu`,
                    },
                    { key: C4, action: `contextToggle:${shiftContext}:1:0` },
                    { key: B9, action: `contextToggle:${shiftContext}:1:0` },
                ]}
                visibilityContext={[{ index: shiftContext, value: 1, condition: 'SHOW_WHEN' }]}
            />
        </>
    );
}
