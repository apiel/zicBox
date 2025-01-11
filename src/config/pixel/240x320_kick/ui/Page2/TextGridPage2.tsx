import * as React from '@/libs/react';

import { TextGridSel } from '../TextGridSel';

export function TextGridPage2({ selected }: { selected: number }) {
    return (
        <TextGridSel
            items={['Amp.', 'Click', '...', '&empty', '&empty', '&icon::musicNote::pixelated']}
            keys={[
                    { key: 'q', action: 'setView:Page2' },
                    { key: 'w', action: 'setView:Click' },
                    { key: 'e', action: 'contextToggle:254:1:0' },

                    // { key: 'a', action: 'setView:Waveform' },
                    // { key: 's', action: 'setView:Frequency' },
                    { key: 'd', action: 'noteOn:Drum23:60' },
            ]}
            selected={selected}
            contextValue={0}
        />
    );
}