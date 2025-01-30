import * as React from '@/libs/react';

import { Rect } from '@/libs/components/Rect';
import { TextGridSel } from '../components/TextGridSel';
import { Title } from '../components/Title';
import { btn1, btn2, btn5, btn6, btn7, btnShift, ColorTrack2 } from '../constants';

export function TextGridBass({ selected, viewName }: { selected: number; viewName: string }) {
    return (
        <>
            <Title title="Bass" />
            <Rect position={[70, 28, 6, 6]} color={ColorTrack2} />
            <TextGridSel
                ITEM_BACKGROUND={ColorTrack2}
                items={['Fx1/Fx2', 'Env', '...', 'Seq.', 'Waveform', '&icon::musicNote::pixelated']}
                keys={[
                    { key: btn1, action: viewName === 'Bass' ? 'setView:BassDistortion' : 'setView:Bass' },
                    { key: btn2, action: 'setView:BassEnv' },
                    { key: btnShift, action: 'contextToggle:254:1:0' },

                    { key: btn5, action: 'setView:BassSeq' },
                    { key: btn6, action: 'setView:BassWaveform' },
                    { key: btn7, action: 'noteOn:Bass:60' },
                ]}
                selected={selected}
                contextValue={0}
            />
        </>
    );
}
