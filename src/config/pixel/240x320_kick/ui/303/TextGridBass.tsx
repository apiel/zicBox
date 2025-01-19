import * as React from '@/libs/react';

import { TextGridSel } from '../components/TextGridSel';
import { Title } from '../components/Title';

export function TextGridBass({ selected, viewName }: { selected: number; viewName: string }) {
    return (
        <>
            <Title title="303" />
            <TextGridSel
                items={['Snare', '303', '...', 'HiHat', 'Seq.', '&icon::musicNote::pixelated']}
                keys={[
                    { key: 'q', action: viewName === 'Snare' ? 'setView:Snare2' : 'setView:Snare' },
                    { key: 'w', action: viewName === 'Bass' ? 'setView:Bass2' : 'setView:Bass' },
                    { key: 'e', action: 'contextToggle:254:1:0' },

                    {
                        key: 'a',
                        action: viewName === 'HiHat' ? 'setView:HiHat2' : 'setView:HiHat',
                    },
                    // { key: 's', action: 'setView:Frequency' },
                    { key: 'd', action: 'noteOn:303:60' },
                ]}
                selected={selected}
                contextValue={0}
            />
        </>
    );
}
