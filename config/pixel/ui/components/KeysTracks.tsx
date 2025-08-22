import * as React from '@/libs/react';

import { HiddenValue } from '@/libs/nativeComponents/HiddenValue';
import { A1, A2, A3, A4, B1, B2, B3, B4, C1, Drum4Track, shiftContext } from '../constants';

export function pages(viewName: string, baseName: string) {
    if (viewName === baseName) {
        return `setView:${baseName}:page2#track`;
    } else if (viewName === `${baseName}:page2`) {
        return `setView:${baseName}:page3#track`;
    }
    return `setView:${baseName}#track`;
}

export function KeysTracks({
    synthName,
    viewName,
    track,
}: {
    synthName?: string;
    viewName: string;
    track: number;
}) {
    return (
        <HiddenValue
            keys={[
                // { key: C1, action: `noteOn:${synthName}:60` },
                {
                    key: C1,
                    action:
                        track <= Drum4Track
                            ? `noteOnAndRepeat:${synthName}:60:12:${track}`
                            : `noteOn:${synthName}:60`,
                    context: { id: shiftContext, value: 0 },
                },

                { key: A1, action: pages(viewName, `Drum1`) },
                // { key: B1, action: `.toggle:1`, context: { id: shiftContext, value: 0 } },
                // { key: B1, action: `.save:1`, context: { id: shiftContext, value: 1 } },

                { key: A2, action: pages(viewName, `Drum2`) },
                // { key: B2, action: `.toggle:2`, context: { id: shiftContext, value: 0 } },
                // { key: B2, action: `.save:2`, context: { id: shiftContext, value: 1 } },
                // { key: C2, action: `noteOn:Drum2:60` },

                { key: A3, action: pages(viewName, `Drum3`) },
                // { key: B3, action: `.toggle:3`, context: { id: shiftContext, value: 0 } },
                // { key: B3, action: `.save:3`, context: { id: shiftContext, value: 1 } },
                // { key: C3, action: `noteOn:Drum3:60` },

                { key: A4, action: pages(viewName, `Drum4`) },
                // { key: B4, action: `.toggle:4`, context: { id: shiftContext, value: 0 } },
                // { key: B4, action: `.save:4`, context: { id: shiftContext, value: 1 } },
                // { key: C4, action: `noteOn:Drum4:60` },

                { key: B1, action: pages(viewName, `Sample1`) },
                // { key: B5, action: `.toggle:5`, context: { id: shiftContext, value: 0 } },
                // { key: B5, action: `.save:5`, context: { id: shiftContext, value: 1 } },
                // { key: C5, action: `noteOn:Sample1:60` },

                { key: B2, action: pages(viewName, `Sample2`) },
                // { key: B6, action: `.toggle:6`, context: { id: shiftContext, value: 0 } },
                // { key: B6, action: `.save:6`, context: { id: shiftContext, value: 1 } },
                // { key: C6, action: `noteOn:Sample2:60` },

                { key: B3, action: pages(viewName, `Synth1`) },
                // { key: B7, action: `.toggle:7`, context: { id: shiftContext, value: 0 } },
                // { key: B7, action: `.save:7`, context: { id: shiftContext, value: 1 } },
                // { key: C7, action: `noteOn:Sample3:60` },

                { key: B4, action: pages(viewName, `Synth2`) },
                // { key: B8, action: `.toggle:8`, context: { id: shiftContext, value: 0 } },
                // { key: B8, action: `.save:8`, context: { id: shiftContext, value: 1 } },
                // { key: C8, action: `noteOn:Sample4:60` },
            ]}
        />
    );
}
