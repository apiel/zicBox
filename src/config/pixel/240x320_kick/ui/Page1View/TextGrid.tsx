{
    /* progressbar */
}
import * as React from '@/libs/react';

import { Keymaps } from '@/libs/components/Keymaps';
import { TextGrid } from '@/libs/components/TextGrid';
import { VisibilityContext } from '@/libs/components/VisibilityContext';
import { KeyInfoPosition } from '@/pixel/240x320_kick/ui/constants';

export function TextGridMain() {
    return (
        <TextGrid
            position={KeyInfoPosition}
            rows={['Main Dist. ...', 'Waveform Freq. &icon::musicNote::pixelated']}
        >
            <VisibilityContext index={254} condition="SHOW_WHEN" value={0} />
            <Keymaps
                keys={[
                    { key: 'w', action: 'setView:SaveTape' },
                    { key: 's', action: 'setView:Home' },
                    { key: 'e', action: 'contextToggle:254:1:0' },

                    { key: 'd', action: 'noteOn:Drum23:60' },
                ]}
            />
        </TextGrid>
    );
}
