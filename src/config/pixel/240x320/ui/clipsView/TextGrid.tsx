{
    /* progressbar */
}
import * as React from '@/libs/react';

import { Keymaps } from '@/libs/components/Keymaps';
import { TextGrid } from '@/libs/components/TextGrid';
import { VisibilityContext } from '@/libs/components/VisibilityContext';
import { KeyInfoPosition } from '@/pixel/240x320/ui/constants';

export function TextGridClips() {
    return (
        <TextGrid
            position={KeyInfoPosition}
            rows={[
                '&icon::toggle::rect &icon::arrowUp::filled ...',
                '&icon::arrowLeft::filled &icon::arrowDown::filled &icon::arrowRight::filled',
            ]}
        >
            <VisibilityContext index={254} condition="SHOW_WHEN" value={0} />
            <Keymaps
                keys={[
                    { key: 'e', action: 'contextToggle:254:1:0' },
                    { key: 'a', action: 'incGroup:-1' },
                    { key: 'd', action: 'incGroup:+1' },
                ]}
            />
        </TextGrid>
    );
}

export function TextGridClipsShifted() {
    return (
        <TextGrid
            position={KeyInfoPosition}
            rows={['Next &icon::play::filled ^...', 'Home &icon::trash Save']}
        >
            <VisibilityContext index={254} condition="SHOW_WHEN" value={1} />
            <Keymaps
                keys={[
                    { key: 'w', action: 'playPause' },
                    { key: 'e', action: 'contextToggle:254:1:0' },
                    { key: 'a', action: 'setView:Home' },
                ]}
            />
        </TextGrid>
    );
}
