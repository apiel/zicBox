import * as React from '@/libs/react';

import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { KeyInfoPosition } from '../constants';

export function TextGridClips() {
    return (
        <TextGrid
            bounds={KeyInfoPosition}
            rows={[
                '&icon::toggle::rect &icon::arrowUp::filled ...',
                '&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled',
            ]}
            keys={[
                { key: 'e', action: 'contextToggle:254:1:0' },
                { key: 'a', action: 'incGroup:-1' },
                { key: 'd', action: 'incGroup:+1' },
            ]}
            contextValue={0}
        />
    );
}

export function TextGridClipsShifted() {
    return (
        <TextGrid
            bounds={KeyInfoPosition}
            rows={['Menu Save ^...', '&icon::trash Back &icon::play::filled']}
            keys={[
                { key: 'd', action: 'playPause' },
                { key: 'e', action: 'contextToggle:254:1:0' },
                { key: 'q', action: 'setView:Menu' },
            ]}
            contextValue={1}
        />
    );
}
