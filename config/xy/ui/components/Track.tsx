import * as React from '@/libs/react';

import { HiddenValue } from '@/libs/nativeComponents/HiddenValue';
import {
    A1,
    A2,
    A4,
    A5,
    B1,
    B2,
    B3,
    B4,
    B5,
    B6,
    B7,
    B8,
    shiftContext,
    Track1,
    Track2,
    Track3,
    Track4,
    Track5,
    Track6,
    Track7,
    Track8,
} from '../constants';

export function pages(viewName: string, baseName: string) {
    if (viewName === baseName) {
        return `setView:${baseName}:page2#track`;
    } else if (viewName === `${baseName}:page2`) {
        return `setView:${baseName}:page3#track`;
    } else if (viewName === `${baseName}:page3`) {
        return `setView:${baseName}:page4#track`;
    }
    return `setView:${baseName}#track`;
}

function getKeys(key: string | number, track: number, viewName: string, baseName: string) {
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
                        action: `noteOn:${synthName}:60`,
                        context: { id: shiftContext, value: 0 },
                    },

                    ...getKeys(B1, Track1, viewName, `Track1`),
                    ...getKeys(B2, Track2, viewName, `Track2`),
                    ...getKeys(B3, Track3, viewName, `Track3`),
                    ...getKeys(B4, Track4, viewName, `Track4`),
                    ...getKeys(B5, Track5, viewName, `Track5`),
                    ...getKeys(B6, Track6, viewName, `Track6`),
                    ...getKeys(B7, Track7, viewName, `Track7`),
                    ...getKeys(B8, Track8, viewName, `Track8`),
                ]}
            />

            <HiddenValue
                keys={[
                    { key: A2, action: pages(viewName, synthName!) },
                    // A3 should be switch pages again...
                    { key: A4, action: `setView:${synthName}Seq` },
                    { key: A5, action: `contextToggle:${shiftContext}:1:0` },
                ]}
                visibilityContext={[{ index: shiftContext, value: 1, condition: 'SHOW_WHEN_NOT' }]}
            />

            <HiddenValue // When shifted
                keys={[
                    { key: A1, action: `playPause` },
                    {
                        key: A2,
                        action: `contextToggle:${shiftContext}:1:0`,
                        action2: `setView:Menu`,
                    },
                    { key: A5, action: `contextToggle:${shiftContext}:1:0` },
                ]}
                visibilityContext={[{ index: shiftContext, value: 1, condition: 'SHOW_WHEN' }]}
            />
        </>
    );
}
