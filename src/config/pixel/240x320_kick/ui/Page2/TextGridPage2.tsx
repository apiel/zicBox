import * as React from '@/libs/react';

import { Keymaps } from '@/libs/components/Keymaps';
import { TextGrid } from '@/libs/components/TextGrid';
import { VisibilityContext } from '@/libs/components/VisibilityContext';
import { KeyInfoPosition } from '@/pixel/240x320_kick/ui/constants';

export function TextGridPage2() {
    return (
        <TextGrid
            position={KeyInfoPosition}
            rows={['Amp. Click ...', '&empty &empty &icon::musicNote::pixelated']}
        >
            <VisibilityContext index={254} condition="SHOW_WHEN" value={0} />
            <Keymaps
                keys={[
                    { key: 'q', action: 'setView:Page2' },
                    { key: 'w', action: 'setView:Click' },
                    { key: 'e', action: 'contextToggle:254:1:0' },

                    // { key: 'a', action: 'setView:Waveform' },
                    // { key: 's', action: 'setView:Frequency' },
                    { key: 'd', action: 'noteOn:Drum23:60' },
                ]}
            />
        </TextGrid>
    );
}
