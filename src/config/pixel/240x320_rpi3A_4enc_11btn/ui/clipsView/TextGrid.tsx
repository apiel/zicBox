import * as React from '@/libs/react';

import { Keymaps } from '@/libs/nativeComponents/Keymaps';
import { TextGrid } from '@/libs/nativeComponents/TextGrid';
import { VisibilityContext } from '@/libs/nativeComponents/VisibilityContext';
import { btn1, btn5, btn7, btnShift, KeyInfoPosition } from '../constants';

export function TextGridClips() {
    return (
        <TextGrid
            bounds={KeyInfoPosition}
            rows={[
                '&icon::toggle::rect &icon::arrowUp::filled ...',
                '&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled',
            ]}
        >
            <VisibilityContext index={254} condition="SHOW_WHEN" value={0} />
            <Keymaps
                keys={[
                    { key: btnShift, action: 'contextToggle:254:1:0' },
                    { key: btn5, action: 'incGroup:-1' },
                    { key: btn7, action: 'incGroup:+1' },
                ]}
            />
        </TextGrid>
    );
}

export function TextGridClipsShifted() {
    return (
        <TextGrid
            bounds={KeyInfoPosition}
            rows={['Menu Save ^...', '&icon::trash Back &icon::play::filled']}
        >
            <VisibilityContext index={254} condition="SHOW_WHEN" value={1} />
            <Keymaps
                keys={[
                    { key: btn7, action: 'playPause' },
                    { key: btnShift, action: 'contextToggle:254:1:0' },
                    { key: btn1, action: 'setView:Menu' },
                ]}
            />
        </TextGrid>
    );
}
