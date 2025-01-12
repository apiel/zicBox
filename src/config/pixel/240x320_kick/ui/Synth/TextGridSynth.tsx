import * as React from '@/libs/react';

import { TextGridSel } from '../components/TextGridSel';

export function TextGridSynth({ selected }: { selected: number }) {
    return (
        <TextGridSel
            items={['Fx1/Fx2', 'Waveform', '...', 'Amp/Click', 'Freq.', '&icon::musicNote::pixelated']}
            keys={[
                    { key: 'q', action: selected === 0 ? 'setView:Distortion' : 'setView:Master' },
                    { key: 'w', action: 'setView:Waveform' },
                    { key: 'e', action: 'contextToggle:254:1:0' },

                    { key: 'a', action: selected === 3 ? 'setView:Click' : 'setView:Amplitude' },
                    { key: 's', action: 'setView:Frequency' },
                    { key: 'd', action: 'noteOn:Drum23:60' },
            ]}
            selected={selected}
            contextValue={0}
        />
    );
}
