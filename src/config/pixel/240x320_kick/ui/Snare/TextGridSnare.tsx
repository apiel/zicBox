import * as React from '@/libs/react';

import { TextGridSel } from '../components/TextGridSel';

export function TextGridSnare({ selected, viewName }: { selected: number, viewName: string }) {
    return (
        <TextGridSel
            items={['Fx1/Fx2', 'Waveform', '...', 'Amp/Click', 'Freq.', '&icon::musicNote::pixelated']}
            keys={[
                    { key: 'q', action: viewName === 'Master' ? 'setView:Distortion' : 'setView:Master' },
                    { key: 'w', action: 'setView:Waveform' },
                    { key: 'e', action: 'contextToggle:254:1:0' },

                    { key: 'a', action: viewName === 'Amplitude' ? 'setView:Click' : 'setView:Amplitude' },
                    { key: 's', action: 'setView:Frequency' },
                    { key: 'd', action: 'noteOn:Drum23:60' },
            ]}
            selected={selected}
            contextValue={0}
        />
    );
}
