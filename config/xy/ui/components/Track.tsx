import * as React from '@/libs/react';

import { HiddenValue } from '@/libs/nativeComponents/HiddenValue';
import { B1, B2, B3, B4, B5, B6, B7, B8, shiftContext, Track1, Track2, Track3, Track4, Track5, Track6, Track7, Track8 } from '../constants';

function getKeys(key: string | number, track: number, viewName: string, baseName: string) {
    return viewName.startsWith(baseName)
        ? [
            // Here we could do some action if track is already active
            //   {
            //       key,
            //       action: `contextToggle:${shiftContext}:2:0`,
            //       context: { id: shiftContext, value: 2 },
            //   },
            //   {
            //       key,
            //       action: `contextToggle:${shiftContext}:2:0`,
            //       context: { id: shiftContext, value: 0 },
            //   },
          ]
        : [
              {
                  key,
                  action: `setView:${baseName}#track`,
                  action2: `setActiveTrack:${track}`,
                  context: { id: shiftContext, value: 0 },
              },
          ];
}

export function Track({ viewName }: { viewName: string }) {
    return (
        <>
            <HiddenValue
                keys={[
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
        </>
    );
}
