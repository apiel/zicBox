import * as React from '@/libs/react';

import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { btn1, btn5, btn6, btn7, btnShift, KeyInfoPosition } from '../constants';

export function TextGridClips() {
    return (
        <TextGrid
            bounds={KeyInfoPosition}
            rows={[
                '&icon::toggle::rect &icon::arrowUp::filled ...',
                '&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled',
            ]}
            keys={[
                { key: btnShift, action: 'contextToggle:254:1:0' },
                { key: btn5, action: 'incGroup:-1' },
                { key: btn7, action: 'incGroup:+1' },
            ]}
            contextValue={[0]}
        />
    );
}

export function TextGridClipsShifted() {
    return (
        <TextGrid
            bounds={KeyInfoPosition}
            rows={['Menu Save ^...', '&icon::trash Back &icon::play::filled']}
            keys={[
                { key: btn6, action: 'setView:Kick' },
                { key: btn7, action: 'playPause' },
                { key: btnShift, action: 'contextToggle:254:1:0' },
                { key: btn1, action: 'setView:Menu' },
            ]}
            contextValue={[1]}
        />
    );
}
