import * as React from '@/libs/react';

import { HiddenValue } from '@/libs/nativeComponents/HiddenValue';
import { darken } from '@/libs/ui';
import { A1, A10, A2, A3, A4, A5, A6, A7, A8, A9, B1, B10, B2, B3, B4, B5, B6, B7, B8, B9, C1, C10, C2, C3, C4, C5, C6, C7, C8, C9, ColorBtnOff, ColorTrack1, ColorTrack10, ColorTrack2, ColorTrack3, ColorTrack4, ColorTrack5, ColorTrack6, ColorTrack7, ColorTrack8, ColorTrack9, D1, D10, D2, D3, D4, D5, D6, D7, D8, D9, shiftContext } from '../constants';

function getVariantColor(viewName: string) {
    if (viewName.startsWith(`Drum1`)) {
        return ColorTrack1;
    }
    if (viewName.startsWith(`Drum2`)) {
        return ColorTrack2;
    }
    if (viewName.startsWith(`Drum3`)) {
        return ColorTrack3;
    }
    if (viewName.startsWith(`Drum4`)) {
        return ColorTrack4;
    }
    if (viewName.startsWith(`Synth1`)) {
        return ColorTrack5;
    }
    if (viewName.startsWith(`Synth2`)) {
        return ColorTrack6;
    }
    if (viewName.startsWith(`Synth3`)) {
        return ColorTrack7;
    }
    if (viewName.startsWith(`Synth4`)) {
        return ColorTrack8;
    }
    if (viewName.startsWith(`Synth5`)) {
        return ColorTrack9;
    }
    if (viewName.startsWith(`Synth6`)) {
        return ColorTrack10;
    }
    return ColorBtnOff;
}

export function KeysTracks({ synthName, viewName }: { synthName?: string; viewName: string }) {
    const variantColor = getVariantColor(viewName);
    return (
        <HiddenValue
            keys={[
                { key: A1, action: viewName === `Drum1` ? `setView:Drum1:page2#track` : `setView:Drum1#track` },
                { key: B1, action: `.toggle:1`, context: { id: shiftContext, value: 0 } },
                { key: B1, action: `.save:1`, context: { id: shiftContext, value: 1 } },
                { key: C1, action: `noteOn:Drum1:60` },

                { key: A2, action: `setView:Drum2#track` },
                { key: B2, action: `.toggle:2`, context: { id: shiftContext, value: 0 } },
                { key: B2, action: `.save:2`, context: { id: shiftContext, value: 1 } },
                { key: C2, action: `noteOn:Drum2:60` },

                { key: A3, action: `setView:Drum3#track` },
                { key: B3, action: `.toggle:3`, context: { id: shiftContext, value: 0 } },
                { key: B3, action: `.save:3`, context: { id: shiftContext, value: 1 } },
                { key: C3, action: `noteOn:Drum3:60` },

                { key: A4, action: `setView:Drum4#track` },
                { key: B4, action: `.toggle:4`, context: { id: shiftContext, value: 0 } },
                { key: B4, action: `.save:4`, context: { id: shiftContext, value: 1 } },
                { key: C4, action: `noteOn:Drum4:60` },

                { key: A5, action: viewName === `Synth1` ? `setView:Synth1:page2#track` : `setView:Synth1#track` },
                { key: B5, action: `.toggle:5`, context: { id: shiftContext, value: 0 } },
                { key: B5, action: `.save:5`, context: { id: shiftContext, value: 1 } },
                { key: C5, action: `noteOn:Synth1:60` },

                { key: A6, action: viewName === `Synth2` ? `setView:Synth2:page2#track` : `setView:Synth2#track` },
                { key: B6, action: `.toggle:6`, context: { id: shiftContext, value: 0 } },
                { key: B6, action: `.save:6`, context: { id: shiftContext, value: 1 } },
                { key: C6, action: `noteOn:Synth2:60` },

                { key: A7, action: viewName === `Synth3` ? `setView:Synth3:page2#track` : `setView:Synth3#track` },
                { key: B7, action: `.toggle:7`, context: { id: shiftContext, value: 0 } },
                { key: B7, action: `.save:7`, context: { id: shiftContext, value: 1 } },
                { key: C7, action: `noteOn:Synth3:60` },

                { key: A8, action: `setView:Sample1#track` },
                { key: B8, action: `.toggle:8`, context: { id: shiftContext, value: 0 } },
                { key: B8, action: `.save:8`, context: { id: shiftContext, value: 1 } },
                { key: C8, action: `noteOn:Sample1:60` },

                { key: A9, action: `setView:Sample2#track` },
                { key: B9, action: `.toggle:9`, context: { id: shiftContext, value: 0 } },
                { key: B9, action: `.save:9`, context: { id: shiftContext, value: 1 } },
                { key: C9, action: `noteOn:Sample2:60` },

                { key: A10, action: `setView:Sample3#track` },
                { key: B10, action: `.toggle:10`, context: { id: shiftContext, value: 0 } },
                { key: B10, action: `.save:10`, context: { id: shiftContext, value: 1 } },
                { key: C10, action: `noteOn:Sample3:60` },
            ]}
            controllerColors={[{ controller: 'Default', colors:[
                { key: A1, color: viewName.startsWith(`Drum1`) ? ColorTrack1 : darken(ColorTrack1, 0.9) },
                { key: B1, color: darken(variantColor, 0.9) },
                { key: C1, color: darken(ColorTrack1, 0.9) },
                { key: D1, color: ColorBtnOff },

                { key: A2, color: viewName.startsWith(`Drum2`) ? ColorTrack2 : darken(ColorTrack2, 0.9) },
                { key: B2, color: darken(variantColor, 0.9) },
                { key: C2, color: darken(ColorTrack2, 0.9) },
                { key: D2, color: ColorBtnOff },

                { key: A3, color: viewName.startsWith(`Drum3`) ? ColorTrack3 : darken(ColorTrack3, 0.9) },
                { key: B3, color: darken(variantColor, 0.9) },
                { key: C3, color: darken(ColorTrack3, 0.9) },
                { key: D3, color: ColorBtnOff },

                { key: A4, color: viewName.startsWith(`Drum4`) ? ColorTrack4 : darken(ColorTrack4, 0.9) },
                { key: B4, color: darken(variantColor, 0.9) },
                { key: C4, color: darken(ColorTrack4, 0.9) },
                { key: D4, color: ColorBtnOff },

                { key: A5, color: viewName.startsWith(`Synth1`) ? ColorTrack5 : darken(ColorTrack5, 0.9) },
                { key: B5, color: darken(variantColor, 0.9) },
                { key: C5, color: darken(ColorTrack5, 0.9) },
                { key: D5, color: ColorBtnOff },

                { key: A6, color: viewName.startsWith(`Synth2`) ? ColorTrack6 : darken(ColorTrack6, 0.9) },
                { key: B6, color: darken(variantColor, 0.9) },
                { key: C6, color: darken(ColorTrack6, 0.9) },
                { key: D6, color: ColorBtnOff },

                { key: A7, color: viewName.startsWith(`Synth3`) ? ColorTrack7 : darken(ColorTrack7, 0.9) },
                { key: B7, color: darken(variantColor, 0.9) },
                { key: C7, color: darken(ColorTrack7, 0.9) },
                { key: D7, color: ColorBtnOff },

                { key: A8, color: viewName.startsWith(`Sample1`) ? ColorTrack8 : darken(ColorTrack8, 0.9) },
                { key: B8, color: darken(variantColor, 0.9) },
                { key: C8, color: darken(ColorTrack8, 0.9) },
                { key: D8, color: ColorBtnOff },

                { key: A9, color: viewName.startsWith(`Sample2`) ? ColorTrack9 : darken(ColorTrack9, 0.9) },
                { key: B9, color: darken(variantColor, 0.9) },
                { key: C9, color: darken(ColorTrack9, 0.9) },
                { key: D9, color: ColorBtnOff },

                { key: A10, color: viewName.startsWith(`Sample3`) ? ColorTrack10 : darken(ColorTrack10, 0.9) },
                { key: B10, color: darken(variantColor, 0.9) },
                { key: C10, color: darken(ColorTrack10, 0.9) },
                { key: D10, color: ColorBtnOff },

            ] }]}
        />
    );
}
