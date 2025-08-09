import * as React from '@/libs/react';

import { HiddenValue } from '@/libs/nativeComponents/HiddenValue';
import { D1, D10, D2, D3, D4, D5, D6, D7, D8, D9 } from '../constants';

export function KeysScatter({seqTrack, scatterTrack}: {seqTrack: number, scatterTrack: number}) {
    return (
        <HiddenValue
            keys={[
                { key: D1, action: `noteRepeat:Sequencer:60:1:${seqTrack}` },
                { key: D2, action: `noteOn:Scatter${scatterTrack}:61` },
                { key: D3, action: `noteOn:Scatter${scatterTrack}:62` },
                { key: D4, action: `noteOn:Scatter${scatterTrack}:63` },
                { key: D5, action: `noteOn:Scatter${scatterTrack}:64` },
                { key: D6, action: `noteOn:Scatter${scatterTrack}:65` },
                { key: D7, action: `noteOn:Scatter${scatterTrack}:66` },
                { key: D8, action: `noteOn:Scatter${scatterTrack}:67` },
                { key: D9, action: `noteOn:Scatter${scatterTrack}:68` },
                { key: D10, action: `noteOn:Scatter${scatterTrack}:69` },
            ]}
            controllerColors={[
                {
                    controller: 'Default',
                    colors: [
                        { key: D1, color: 'primary' },
                        { key: D2, color: 'primary' },
                        { key: D3, color: 'primary' },
                        { key: D4, color: 'primary' },
                        { key: D5, color: 'primary' },
                        { key: D6, color: 'primary' },
                        { key: D7, color: 'primary' },
                        { key: D8, color: 'primary' },
                        { key: D9, color: 'primary' },
                        { key: D10, color: 'primary' },
                    ],
                },
            ]}
        />
    );
}
