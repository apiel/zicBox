import * as React from '@/libs/react';

import { TextGridSel } from '../TextGridSel';

export function TextGridPage1({ selected }: { selected: number }) {
    return (
        <TextGridSel
            items={['Main', 'Dist.', '...', 'Waveform', 'Freq.', '&icon::musicNote::pixelated']}
            keys={[
                    { key: 'q', action: 'setView:Page1' },
                    { key: 'w', action: 'setView:Distortion' },
                    { key: 'e', action: 'contextToggle:254:1:0' },

                    { key: 'a', action: 'setView:Waveform' },
                    { key: 's', action: 'setView:Frequency' },
                    { key: 'd', action: 'noteOn:Drum23:60' },
            ]}
            selected={selected}
            contextValue={0}
        />
    );
}
