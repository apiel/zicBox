import * as React from '@/libs/react';

import { TextGridSel } from './TextGridSel';

export function TextGridCommon({ selected }: { selected: number }) {
    return (
        <TextGridSel
            items={['Seq.', 'Synth', '^...', '&empty', 'Menu', '&icon::play::filled']}
            keys={[
                { key: 'q', action: 'setView:Sequencer' },
                { key: 'w', action: `setView:Synth` },
                { key: 'e', action: 'contextToggle:254:1:0' },
                // { key: 'a', action: 'setView:Sequencer' },
                { key: 's', action: 'setView:Menu' },
                { key: 'd', action: 'playPause' },
            ]}
            selected={selected}
            contextValue={1}
        />
    );
}
