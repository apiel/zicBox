import * as React from '@/libs/react';

import { Clips } from '@/libs/nativeComponents/Clips';
import { HiddenValue } from '@/libs/nativeComponents/HiddenValue';
import {
    A1,
    A2,
    A3,
    A4,
    B1,
    B2,
    B3,
    B4,
    C1,
    C2,
    C3,
    C4,
    clipRenderContext,
    Drum1Track,
    Drum2Track,
    Drum3Track,
    Drum4Track,
    Sample1Track,
    Sample2Track,
    shiftContext,
    Synth1Track,
    Synth2Track,
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
              // { key, action: pages(viewName, `Drum1`), context: { id: shiftContext, value: 0 } },
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
            <Clips
                bounds={[152, 2, 170, 15]}
                track={track}
                color={color}
                visibleCount={8}
                renderContextId={clipRenderContext}
                keys={[
                    {
                        key: C3,
                        action: `audioEvent:SAVE_VARIATION`,
                        action2: `.message:All clips saved`,
                        context: { id: shiftContext, value: 1 },
                    },
                ]}
            />
            <HiddenValue
                keys={[
                    // { key: C1, action: `noteOn:${synthName}:60` },
                    {
                        key: C1,
                        action:
                            track <= Drum4Track
                                ? `noteOnAndRepeat:${synthName}:60:val:${track}`
                                : `noteOn:${synthName}:60`,
                        context: { id: shiftContext, value: 0 },
                    },

                    ...getKeys(A1, Drum1Track, viewName, `Drum1`),
                    ...getKeys(A2, Drum2Track, viewName, `Drum2`),
                    ...getKeys(A3, Drum3Track, viewName, `Drum3`),
                    ...getKeys(A4, Drum4Track, viewName, `Drum4`),
                    ...getKeys(B1, Synth1Track, viewName, `Synth1`),
                    ...getKeys(B2, Synth2Track, viewName, `Synth2`),
                    ...getKeys(B3, Sample1Track, viewName, `Sample1`),
                    ...getKeys(B4, Sample2Track, viewName, `Sample2`),
                ]}
            />

            <HiddenValue
                keys={[
                    { key: C2, action: pages(viewName, synthName!) },
                    { key: C3, action: `setView:${synthName}Clips` },
                    { key: C4, action: `contextToggle:${shiftContext}:1:0` },
                ]}
                visibilityContext={[{ index: shiftContext, value: 1, condition: 'SHOW_WHEN_NOT' }]}
            />

            <HiddenValue // When shifted
                keys={[
                    { key: C1, action: `playPause` },
                    {
                        key: C2,
                        action: `contextToggle:${shiftContext}:1:0`,
                        action2: `setView:Menu`,
                    },
                    // { key: C2, action: `setContext:${shiftContext}:0`, action2: `setView:Menu` },
                    // { key: C3, action: `audioEvent:RELOAD_VARIATION` },
                    { key: C4, action: `contextToggle:${shiftContext}:1:0` },
                ]}
                visibilityContext={[{ index: shiftContext, value: 1, condition: 'SHOW_WHEN' }]}
            />
        </>
    );
}
