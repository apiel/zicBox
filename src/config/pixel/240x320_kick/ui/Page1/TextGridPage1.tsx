import * as React from '@/libs/react';

import { Keymaps } from '@/libs/components/Keymaps';
import { TextGrid } from '@/libs/components/TextGrid';
import { VisibilityContext } from '@/libs/components/VisibilityContext';
import { KeyInfoPosition } from '@/pixel/240x320_kick/ui/constants';

export function TextGridPage1({ selected }: { selected: number }) {
    const items = ['Main', 'Dist.', '...', 'Waveform', 'Freq.', '&icon::musicNote::pixelated'];
    if (selected >= 0) {
        items[selected] = `^${items[selected]}`;
    }
    return (
        <TextGrid
            position={KeyInfoPosition}
            rows={[`${items[0]} ${items[1]} ${items[2]}`, `${items[3]} ${items[4]} ${items[5]}`]}
        >
            <VisibilityContext index={254} condition="SHOW_WHEN" value={0} />
            <Keymaps
                keys={[
                    { key: 'q', action: 'setView:Page1' },
                    { key: 'w', action: 'setView:Distortion' },
                    { key: 'e', action: 'contextToggle:254:1:0' },

                    { key: 'a', action: 'setView:Waveform' },
                    { key: 's', action: 'setView:Frequency' },
                    { key: 'd', action: 'noteOn:Drum23:60' },
                ]}
            />
        </TextGrid>
    );
}
