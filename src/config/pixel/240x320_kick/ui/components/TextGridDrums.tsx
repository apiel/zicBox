import * as React from '@/libs/react';

import { TextGridSel } from './TextGridSel';
import { Title } from './Title';

export function TextGridDrums({
    selected,
    viewName,
    target,
}: {
    selected: number;
    viewName: string;
    target: string;
}) {
    return (
        <>
            <Title title={target} />
            <TextGridSel
                items={['Snare', 'Sample', '...', 'HiHat', 'Seq.', '&icon::musicNote::pixelated']}
                keys={[
                    { key: 'q', action: viewName === 'Snare' ? 'setView:Snare2' : 'setView:Snare' },
                    { key: 'w', action: viewName === 'Sample' ? 'setView:Sample2' : 'setView:Sample' },
                    { key: 'e', action: 'contextToggle:254:1:0' },

                    {
                        key: 'a',
                        action: viewName === 'HiHat' ? 'setView:HiHat2' : 'setView:HiHat',
                    },
                    { key: 's', action: 'setView:DrumsSeq' },
                    { key: 'd', action: `noteOn:${target}:60` },
                ]}
                selected={selected}
                contextValue={0}
            />
        </>
    );
}
