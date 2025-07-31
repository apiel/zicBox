import * as React from '@/libs/react';

import { HiddenValue } from '@/libs/nativeComponents/HiddenValue';
import { darken } from '@/libs/ui';
import { A1, A10, A2, A3, A4, A5, A6, A7, A8, A9, B1, B10, B2, B3, B4, B5, B6, B7, B8, B9, C1, C10, C2, C3, C4, C5, C6, C7, C8, C9, ColorTrack1, ColorTrack10, ColorTrack2, ColorTrack3, ColorTrack4, ColorTrack5, ColorTrack6, ColorTrack7, ColorTrack8, ColorTrack9, D1, D10, D2, D3, D4, D5, D6, D7, D8, D9 } from '../constants';

function getVariantColor(viewName: string) {
    if (viewName === `Drum1`) {
        return ColorTrack1;
    }
    if (viewName === `Drum2`) {
        return ColorTrack2;
    }
    if (viewName === `Drum3`) {
        return ColorTrack3;
    }
    if (viewName === `Drum4`) {
        return ColorTrack4;
    }
    if (viewName === `Synth1`) {
        return ColorTrack5;
    }
    if (viewName === `Synth2`) {
        return ColorTrack6;
    }
    if (viewName === `Synth3`) {
        return ColorTrack7;
    }
    if (viewName === `Synth4`) {
        return ColorTrack8;
    }
    if (viewName === `Synth5`) {
        return ColorTrack9;
    }
    if (viewName === `Synth6`) {
        return ColorTrack10;
    }
    return ColorTrack1;
}

export function KeysTracks({ synthName, viewName }: { synthName: string; viewName: string }) {
    const variantColor = getVariantColor(viewName);
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

                { key: A5, action: viewName === `Synth1` ? `setView:Synth1:page2` : `setView:Synth1` },
                { key: B5, action: `noteOn:Synth1:60` },

                { key: A6, action: viewName === `Synth2` ? `setView:Synth2:page2` : `setView:Synth2` },
                { key: B6, action: `noteOn:Synth2:60` },

                { key: A7, action: viewName === `Synth3` ? `setView:Synth3:page2` : `setView:Synth3` },
                { key: B7, action: `noteOn:Synth3:60` },
            ]}
            controllerColors={[{ controller: 'Default', colors:[
                { key: A1, color: viewName === `Drum1` ? ColorTrack1 : darken(ColorTrack1, 0.9) },
                { key: B1, color: darken(ColorTrack1, 0.9) },
                // { key: D1, color: darken(ColorTrack1, 0.9) },

                { key: A2, color: viewName === `Drum2` ? ColorTrack2 : darken(ColorTrack2, 0.9) },
                { key: B2, color: darken(ColorTrack2, 0.9) },
                // { key: D2, color: darken(ColorTrack2, 0.9) },

                { key: A3, color: viewName === `Drum3` ? ColorTrack3 : darken(ColorTrack3, 0.9) },
                { key: B3, color: darken(ColorTrack3, 0.9) },
                // { key: D3, color: darken(ColorTrack3, 0.9) },

                { key: A4, color: viewName === `Drum4` ? ColorTrack4 : darken(ColorTrack4, 0.9) },
                { key: B4, color: darken(ColorTrack4, 0.9) },
                // { key: D4, color: darken(ColorTrack4, 0.9) },

                { key: A5, color: viewName === `Synth1` ? ColorTrack5 : darken(ColorTrack5, 0.9) },
                { key: B5, color: darken(ColorTrack5, 0.9) },
                // { key: D5, color: darken(ColorTrack5, 0.9) },

                { key: A6, color: viewName === `Synth2` ? ColorTrack6 : darken(ColorTrack6, 0.9) },
                { key: B6, color: darken(ColorTrack6, 0.9) },
                // { key: D6, color: darken(ColorTrack6, 0.9) },

                { key: A7, color: viewName === `Synth3` ? ColorTrack7 : darken(ColorTrack7, 0.9) },
                { key: B7, color: darken(ColorTrack7, 0.9) },
                // { key: D7, color: darken(ColorTrack7, 0.9) },

                { key: A8, color: viewName === `Sample1` ? ColorTrack8 : darken(ColorTrack8, 0.9) },
                { key: B8, color: darken(ColorTrack8, 0.9) },
                // { key: D8, color: darken(ColorTrack8, 0.9) },

                { key: A9, color: viewName === `Sample2` ? ColorTrack9 : darken(ColorTrack9, 0.9) },
                { key: B9, color: darken(ColorTrack9, 0.9) },
                // { key: D9, color: darken(ColorTrack9, 0.9) },

                { key: A10, color: viewName === `Sample3` ? ColorTrack10 : darken(ColorTrack10, 0.9) },
                { key: B10, color: darken(ColorTrack10, 0.9) },
                // { key: D10, color: darken(ColorTrack10, 0.9) },

                // { key: C1, color: darken(variantColor, 0.9) },
                { key: C1, color: "#000000" },
                { key: D1, color: darken(variantColor, 0.9) },
                // { key: C2, color: darken(variantColor, 0.9) },
                { key: C2, color: "#000000" },
                { key: D2, color: darken(variantColor, 0.9) },
                // { key: C3, color: darken(variantColor, 0.9) },
                { key: C3, color: "#000000" },
                { key: D3, color: darken(variantColor, 0.9) },
                // { key: C4, color: darken(variantColor, 0.9) },
                { key: C4, color: "#000000" },
                { key: D4, color: darken(variantColor, 0.9) },
                // { key: C5, color: darken(variantColor, 0.9) },
                { key: C5, color: "#000000" },
                { key: D5, color: darken(variantColor, 0.9) },
                // { key: C6, color: darken(variantColor, 0.9) },
                { key: C6, color: "#000000" },
                { key: D6, color: darken(variantColor, 0.9) },
                // { key: C7, color: darken(variantColor, 0.9) },
                { key: C7, color: "#000000" },
                { key: D7, color: darken(variantColor, 0.9) },
                // { key: C8, color: darken(variantColor, 0.9) },
                { key: C8, color: "#000000" },
                { key: D8, color: darken(variantColor, 0.9) },
                // { key: C9, color: darken(variantColor, 0.9) },
                { key: C9, color: "#000000" },
                { key: D9, color: darken(variantColor, 0.9) },
                // { key: C10, color: darken(variantColor, 0.9) },
                { key: C10, color: "#000000" },
                { key: D10, color: darken(variantColor, 0.9) },
            ] }]}
        />
    );
}
