import * as React from '@/libs/react';

import { TextGridSel } from '../components/TextGridSel';
import { Title } from '../components/Title';

export function TextGridSnare({ selected, viewName }: { selected: number; viewName: string }) {
    return (
        <>
            <Title title="Snare" />
            <TextGridSel
                items={[
                    'Snare',
                    'Seq.',
                    '...',
                    'Amp/Click',
                    'Freq.',
                    '&icon::musicNote::pixelated',
                ]}
                keys={[
                    { key: 'q', action: viewName === 'Snare' ? 'setView:Snare2' : 'setView:Snare' },
                    { key: 'w', action: 'setView:Snare2' },
                    { key: 'e', action: 'contextToggle:254:1:0' },

                    {
                        key: 'a',
                        action: viewName === 'Amplitude' ? 'setView:Click' : 'setView:Amplitude',
                    },
                    { key: 's', action: 'setView:Frequency' },
                    { key: 'd', action: 'noteOn:Snare:60' },
                ]}
                selected={selected}
                contextValue={0}
            />
        </>
    );
}
