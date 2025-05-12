import * as React from '@/libs/react';

import { VisibilityContext } from '@/libs/nativeComponents/component';
import { Rect } from '@/libs/nativeComponents/Rect';
import { Text } from '@/libs/nativeComponents/Text';
import { rgb } from '@/libs/ui';
import {
    btnDown,
    btnShift,
    btnUp,
    ColorTrack1,
    ColorTrack2,
    ColorTrack3,
    ColorTrack4,
    ColorTrack5,
    ColorTrack6,
    ColorTrack7,
    ColorTrack8,
    KeyInfoPosition,
    KeyTopInfoPosition,
    ScreenWidth
} from '../constants';

export function TextGridClips() {
    const w = ScreenWidth / 4;
    const visibilityContext: VisibilityContext[] = [
        { index: 254, condition: 'SHOW_WHEN', value: 0 },
        { index: 253, condition: 'SHOW_WHEN', value: 0 },
    ];
    const props = {
        visibilityContext,
        font: 'PoppinsLight_8',
        fontSize: 8,
        centered: true,
    };
    return (
        <>
            <Text
                text="Save / Delete"
                bounds={KeyTopInfoPosition}
                color={rgb(192, 192, 192)}
                {...props}
                visibilityContext={[
                    {
                        index: 251,
                        condition: 'SHOW_WHEN',
                        value: 1,
                    },
                ]}
            />
            <Rect
                bounds={KeyTopInfoPosition}
                color="background"
                visibilityContext={[{
                    index: 251,
                    condition: 'SHOW_WHEN',
                    value: 0,
                }]}
            />

            <Text
                text="Kick"
                bounds={[0, KeyInfoPosition[1], w, 12]}
                color={ColorTrack1}
                // bgColor={ColorTrack1}
                {...props}
                keys={[
                    { key: btnUp, action: 'contextToggle:253:1:0' },
                    { key: btnDown, action: 'contextToggle:251:1:0' }, // Should be 251 and not 252, else it will conflict with keyboard toggle
                    { key: btnShift, action: 'contextToggle:254:1:0' },
                ]}
            />
            <Rect
                bounds={[10, KeyInfoPosition[1] + 5, 3, 4]}
                color={ColorTrack1}
                visibilityContext={visibilityContext}
            />
            <Text
                text="Drum1"
                bounds={[w, KeyInfoPosition[1], w, 12]}
                color={ColorTrack2}
                // bgColor={ColorTrack2}
                {...props}
            />
            <Rect
                bounds={[w + 5, KeyInfoPosition[1] + 5, 3, 4]}
                color={ColorTrack2}
                visibilityContext={visibilityContext}
            />
            <Text
                text="Drum2"
                bounds={[2 * w, KeyInfoPosition[1], w, 12]}
                color={ColorTrack3}
                // bgColor={ColorTrack3}
                {...props}
            />
            <Rect
                bounds={[2 * w + 5, KeyInfoPosition[1] + 5, 3, 4]}
                color={ColorTrack3}
                visibilityContext={visibilityContext}
            />
            <Text
                text="Drum3"
                bounds={[3 * w, KeyInfoPosition[1], w, 12]}
                color={ColorTrack4}
                // bgColor={ColorTrack4}
                {...props}
            />
            <Rect
                bounds={[3 * w + 5, KeyInfoPosition[1] + 5, 3, 4]}
                color={ColorTrack4}
                visibilityContext={visibilityContext}
            />

            <Text
                text="Synth1"
                bounds={[0, KeyInfoPosition[1] + 12, w, 12]}
                color={ColorTrack5}
                // bgColor={ColorTrack5}
                {...props}
            />
            <Rect
                bounds={[10, KeyInfoPosition[1] + 17, 3, 4]}
                color={ColorTrack5}
                visibilityContext={visibilityContext}
            />
            <Text
                text="Synth2"
                bounds={[w, KeyInfoPosition[1] + 12, w, 12]}
                color={ColorTrack6}
                // bgColor={ColorTrack6}
                {...props}
            />
            <Rect
                bounds={[w + 5, KeyInfoPosition[1] + 17, 3, 4]}
                color={ColorTrack6}
                visibilityContext={visibilityContext}
            />
            <Text
                text="Sample1"
                bounds={[2 * w, KeyInfoPosition[1] + 12, w, 12]}
                color={ColorTrack7}
                // bgColor={ColorTrack7}
                {...props}
            />
            <Rect
                bounds={[2 * w + 5, KeyInfoPosition[1] + 17, 3, 4]}
                color={ColorTrack7}
                visibilityContext={visibilityContext}
            />
            <Text
                text="Sample2"
                bounds={[3 * w, KeyInfoPosition[1] + 12, w, 12]}
                color={ColorTrack8}
                // bgColor={ColorTrack8}
                {...props}
            />
            <Rect
                bounds={[3 * w + 5, KeyInfoPosition[1] + 17, 3, 4]}
                color={ColorTrack8}
                visibilityContext={visibilityContext}
            />
        </>
    );
}

// export function TextGridClips() {
//     return (
//         <TextGrid
//             bounds={KeyInfoPosition}
//             rows={[
//                 '&icon::toggle::rect &icon::arrowUp::filled &empty &empty',
//                 '&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled &empty',
//             ]}
//             keys={[
//                 { key: btn5, action: 'incGroup:-1' },
//                 { key: btn7, action: 'incGroup:+1' },

//                 { key: btnUp, action: 'contextToggle:253:1:0' },
//                 // { key: btnDown, action: 'contextToggleOnRelease:252:1:0' },
//                 { key: btnShift, action: 'contextToggle:254:1:0' },
//             ]}
//             contextValue={[0]}
//         />
//     );
// }

// export function TextGridClipsShifted() {
//     return (
//         <TextGrid
//             bounds={KeyInfoPosition}
//             rows={['Menu Save ^...', '&icon::trash Back &icon::play::filled']}
//             keys={[
//                 { key: btn6, action: 'setView:Kick' },
//                 { key: btn7, action: 'playPause' },
//                 { key: btnShift, action: 'contextToggle:254:1:0' },
//                 { key: btn1, action: 'setView:Menu' },
//             ]}
//             contextValue={[1]}
//         />
//     );
// }
