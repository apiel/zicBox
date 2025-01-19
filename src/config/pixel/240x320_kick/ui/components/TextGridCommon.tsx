import * as React from '@/libs/react';

import { TextGridSel } from './TextGridSel';

export function TextGridCommon({ selected }: { selected: number }) {
    return (
        <TextGridSel
            items={['Snare/HH.', 'Kick', '^...', 'Snare', 'Menu', '&icon::play::filled']}
            keys={[
                { key: 'q', action: 'setView:Snare' },
                { key: 'w', action: `setView:Drum23` },
                { key: 'e', action: 'contextToggle:254:1:0' },
                { key: 'a', action: 'setView:Snare' },
                { key: 's', action: 'setView:Menu' },
                { key: 'd', action: 'playPause' },
            ]}
            selected={selected}
            contextValue={1}
        />
    );
}
