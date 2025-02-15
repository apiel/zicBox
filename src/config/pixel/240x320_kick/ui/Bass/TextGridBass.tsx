import * as React from '@/libs/react';

import { Rect } from '@/libs/components/Rect';
import { TextGridSel } from '../components/TextGridSel';
import { Title } from '../components/Title';
import { ColorTrack2 } from '../constants';

export function TextGridBass({ selected, viewName }: { selected: number; viewName: string }) {
    return (
        <>
            <Title title="Bass" />
            <Rect bounds={[70, 28, 6, 6]} color={ColorTrack2} />
            <TextGridSel
                ITEM_BACKGROUND={ColorTrack2}
                items={['Fx1/Fx2', 'Env', '...', 'Seq.', 'Waveform', '&icon::musicNote::pixelated']}
                keys={[
                    { key: 'q', action: viewName === 'Bass' ? 'setView:BassDistortion' : 'setView:Bass' },
                    { key: 'w', action: 'setView:BassEnv' },
                    { key: 'e', action: 'contextToggle:254:1:0' },

                    { key: 'a', action: 'setView:BassSeq' },
                    { key: 's', action: 'setView:BassWaveform' },
                    { key: 'd', action: 'noteOn:Bass:60' },
                ]}
                selected={selected}
                contextValue={0}
            />
        </>
    );
}
