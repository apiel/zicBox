import * as React from '@/libs/react';

import { TextGridSel } from '../components/TextGridSel';

export function TextGridDrum23({ selected, viewName }: { selected: number, viewName: string }) {
    return (
        <TextGridSel
            items={['Fx1/Fx2', 'Wave/Freq', '...', 'Amp/Click', 'Sequencer', '&icon::musicNote::pixelated']}
            keys={[
                    { key: 'q', action: viewName === 'Drum23' ? 'setView:Distortion' : 'setView:Drum23' },
                    { key: 'w', action: viewName === 'Waveform' ? 'setView:Frequency' : 'setView:Waveform' },
                    { key: 'e', action: 'contextToggle:254:1:0' },

                    { key: 'a', action: viewName === 'Amplitude' ? 'setView:Click' : 'setView:Amplitude' },
                    { key: 's', action: 'setView:Sequencer' },
                    { key: 'd', action: 'noteOn:Drum23:60' },
            ]}
            selected={selected}
            contextValue={0}
        />
    );
}
