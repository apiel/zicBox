{
    /* progressbar */
}
import * as React from '@/libs/react';

import { Keymap } from '@/libs/components/Keymap';
import { Keymaps } from '@/libs/components/Keymaps';
import { TextGrid } from '@/libs/components/TextGrid';
import { VisibilityContext } from '@/libs/components/VisibilityContext';
import { VisibilityGroup } from '@/libs/components/VisibilityGroup';
import { KeyInfoPosition } from '@/pixel/240x320/ui/constants';

export function TextGridProgressBar() {
    return (
        <TextGrid
            position={KeyInfoPosition}
            rows={[
                '&icon::play::filled &icon::arrowUp::filled ...',
                '&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled',
            ]}
        >
            <VisibilityGroup condition="SHOW_WHEN" group={0} />
            <VisibilityContext index={254} condition="SHOW_WHEN" value={0} />
            <Keymap key="q" action="playPause" />
            <Keymap key="e" action="contextToggle:254:1:0" />
        </TextGrid>
    );
}

export function TextGridProgressBarShifted() {
    return (
        <TextGrid
            position={KeyInfoPosition}
            rows={['Menu &icon::play::filled ^...', 'Clips ? Save']}
        >
            <VisibilityGroup condition="SHOW_WHEN" group={0} />
            <VisibilityContext index={254} condition="SHOW_WHEN" value={1} />
            <Keymaps
                keys={[
                    { key: 'q', action: 'setView:Menu', action2: 'contextToggle:254:1:0' },
                    { key: 'w', action: 'playPause' },
                    { key: 'a', action: 'setView:Clips' },
                    { key: 'e', action: 'contextToggle:254:1:0' },
                ]}
            />
        </TextGrid>
    );
}
