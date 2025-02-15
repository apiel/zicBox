import * as React from '@/libs/react';

import { Keymaps } from '@/libs/nativeComponents/Keymaps';
import { TextGrid } from '@/libs/nativeComponents/TextGrid';
import { VisibilityContext } from '@/libs/nativeComponents/VisibilityContext';
import { KeyInfoPosition } from '@/pixel/240x320_drums/ui/constants';

export function TextGridSeq() {
    return (
        <TextGrid
            bounds={KeyInfoPosition}
            rows={['&icon::arrowUp::filled &icon::toggle::rect ...', '&icon::arrowDown::filled Synth &icon::musicNote::pixelated']}
        >
            <VisibilityContext index={254} condition="SHOW_WHEN" value={0} />
            <Keymaps
                keys={[
                    { key: 'q', action: 'incGroup:-1' },
                    // { key: 'w', action: 'setView:Click' },
                    { key: 'e', action: 'contextToggle:254:1:0' },

                    { key: 'a', action: 'incGroup:+1' },
                    { key: 's', action: 'setView:Drum23' },
                    { key: 'd', action: 'noteOn:Drum23:60' },
                ]}
            />
        </TextGrid>
    );
}
