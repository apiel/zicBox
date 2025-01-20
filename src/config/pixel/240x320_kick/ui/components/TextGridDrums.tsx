import * as React from '@/libs/react';

import { TextGridSel } from './TextGridSel';
import { Title } from './Title';

export function TextGridDrums({
    selected,
    viewName,
    title,
}: {
    selected: number;
    viewName: string;
    title: string;
}) {
    return (
        <>
            <Title title={title} />
            <TextGridSel
                items={['Snare', 'Perc', '...', 'HiHat', 'Seq.', '&icon::musicNote::pixelated']}
                keys={[
                    { key: 'q', action: viewName === 'Snare' ? 'setView:Snare2' : 'setView:Snare' },
                    { key: 'w', action: viewName === 'Perc' ? 'setView:Perc2' : 'setView:Perc' },
                    { key: 'e', action: 'contextToggle:254:1:0' },

                    {
                        key: 'a',
                        action: viewName === 'HiHat' ? 'setView:HiHat2' : 'setView:HiHat',
                    },
                    // { key: 's', action: 'setView:Frequency' },
                    { key: 'd', action: 'noteOn:HiHat:60' },
                ]}
                selected={selected}
                contextValue={0}
            />
        </>
    );
}
