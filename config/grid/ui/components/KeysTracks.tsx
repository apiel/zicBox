import * as React from '@/libs/react';

import { HiddenValue } from '@/libs/nativeComponents/HiddenValue';
import { darken } from '@/libs/ui';
import { A1, A10, A2, A3, A4, A5, A6, A7, A8, A9, B1, B10, B2, B3, B4, B5, B6, B7, B8, B9, ColorTrack1, ColorTrack10, ColorTrack2, ColorTrack3, ColorTrack4, ColorTrack5, ColorTrack6, ColorTrack7, ColorTrack8, ColorTrack9 } from '../constants';

export function KeysTracks({ synthName, viewName }: { synthName: string; viewName: string }) {
    return (
        <HiddenValue
            keys={[
                { key: A1, action: viewName === `Drum1` ? `setView:Drum1:page2` : `setView:Drum1` },
                { key: B1, action: `noteOn:Drum1:60` },

                { key: A2, action: `setView:Drum2` },
                { key: B2, action: `noteOn:Drum2:60` },

                { key: A3, action: `setView:Drum3` },
                { key: B3, action: `noteOn:Drum3:60` },

                { key: A4, action: `setView:Drum4` },
                { key: B4, action: `noteOn:Drum4:60` },

                { key: A5, action: `setView:Synth1` },
                { key: B5, action: `noteOn:Synth1:60` },

                { key: A6, action: `setView:Synth2` },
                { key: B6, action: `noteOn:Synth2:60` },

                { key: A7, action: `setView:Synth3` },
                { key: B7, action: `noteOn:Synth3:60` },
            ]}
            controllerColors={[{ controller: 'Default', colors:[
                { key: A1, color: viewName === `Drum1` ? ColorTrack1 : darken(ColorTrack1, 0.9) },
                { key: B1, color: darken(ColorTrack1, 0.9) },

                { key: A2, color: viewName === `Drum2` ? ColorTrack2 : darken(ColorTrack2, 0.9) },
                { key: B2, color: darken(ColorTrack2, 0.9) },

                { key: A3, color: viewName === `Drum3` ? ColorTrack3 : darken(ColorTrack3, 0.9) },
                { key: B3, color: darken(ColorTrack3, 0.9) },

                { key: A4, color: viewName === `Drum4` ? ColorTrack4 : darken(ColorTrack4, 0.9) },
                { key: B4, color: darken(ColorTrack4, 0.9) },

                { key: A5, color: viewName === `Synth1` ? ColorTrack5 : darken(ColorTrack5, 0.9) },
                { key: B5, color: darken(ColorTrack5, 0.9) },

                { key: A6, color: viewName === `Synth2` ? ColorTrack6 : darken(ColorTrack6, 0.9) },
                { key: B6, color: darken(ColorTrack6, 0.9) },

                { key: A7, color: viewName === `Synth3` ? ColorTrack7 : darken(ColorTrack7, 0.9) },
                { key: B7, color: darken(ColorTrack7, 0.9) },

                { key: A8, color: viewName === `Sample1` ? ColorTrack8 : darken(ColorTrack8, 0.9) },
                { key: B8, color: darken(ColorTrack8, 0.9) },

                { key: A9, color: viewName === `Sample2` ? ColorTrack9 : darken(ColorTrack9, 0.9) },
                { key: B9, color: darken(ColorTrack9, 0.9) },

                { key: A10, color: viewName === `Sample3` ? ColorTrack10 : darken(ColorTrack10, 0.9) },
                { key: B10, color: darken(ColorTrack10, 0.9) },
            ] }]}
        />
    );
}
