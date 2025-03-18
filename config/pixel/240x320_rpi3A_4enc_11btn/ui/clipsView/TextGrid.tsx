import * as React from '@/libs/react';

import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { btn5, btn7, btnShift, btnUp, KeyInfoPosition } from '../constants';

export function TextGridClips() {
    return (
        <TextGrid
            bounds={KeyInfoPosition}
            rows={[
                '&icon::toggle::rect &icon::arrowUp::filled &empty &empty',
                '&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled &empty',
            ]}
            keys={[
                { key: btn5, action: 'incGroup:-1' },
                { key: btn7, action: 'incGroup:+1' },

                { key: btnUp, action: 'contextToggle:253:1:0' },
                // { key: btnDown, action: 'contextToggleOnRelease:252:1:0' },
                { key: btnShift, action: 'contextToggle:254:1:0' },
            ]}
            contextValue={[0]}
        />
    );
}

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
