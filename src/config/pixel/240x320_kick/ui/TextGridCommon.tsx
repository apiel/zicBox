import * as React from '@/libs/react';

import { TextGridSel } from './TextGridSel';

export function TextGridCommon({ selected , page }: { selected: number, page: string }) {
    return (
        <TextGridSel
            items={['&empty', 'Page1/2', '^...', 'Seq.', 'Menu', '&icon::play::filled']}
            keys={[
                // { key: 'q', action: 'setView:Page1' },
                { key: 'w', action: `setView:${page}` },
                { key: 'e', action: 'contextToggle:254:1:0' },
                { key: 'a', action: 'setView:Sequencer' },
                { key: 's', action: 'setView:Menu' },
                { key: 'd', action: 'playPause' },
            ]}
            selected={selected}
            contextValue={1}
        />
    );
}
