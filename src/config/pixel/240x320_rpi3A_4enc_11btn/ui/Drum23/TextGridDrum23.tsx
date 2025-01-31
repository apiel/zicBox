import * as React from '@/libs/react';

import { Rect } from '@/libs/components/Rect';
import { Kick } from '../components/Common';
import { TextGridSel } from '../components/TextGridSel';
import { Title } from '../components/Title';
import { btn1, btn2, btn3, btn4, btn5, btn6, btn7, btn8, btnDown, btnShift, btnUp, ColorTrack1 } from '../constants';

export function TextGridDrum23({ selected, viewName }: { selected: string; viewName: string }) {
    return (
        <>
            <Title title={Kick} />
            <Rect position={[70, 28, 6, 6]} color={ColorTrack1} />
            <TextGridSel
                selectedBackground={ColorTrack1}
                rows={[
                    'Main Fx Wave Freq',
                    'Seq. Amp Click &empty',
                    // '&icon::musicNote::pixelated',
                ]}
                keys={[
                    { key: btn1, action: 'setView:Drum23' }, // viewName === 'Drum23' ? 'setView:Distortion' :
                    { key: btn2, action: 'setView:Distortion' },
                    { key: btn3, action: 'setView:Waveform' },
                    { key: btn4, action: 'setView:Frequency' },

                    { key: btn5, action: 'setView:Sequencer' },
                    { key: btn6, action: 'setView:Amplitude' },
                    { key: btn7, action: 'setView:Click' },
                    { key: btn8, action: 'noteOn:Drum23:60' }, // when not used, let's play noteOn...

                    { key: btnUp, action: 'noteOn:Drum23:60' }, // when not used, let's play noteOn...
                    { key: btnDown, action: 'noteOn:Drum23:60' }, // <--- this should be the default noteOn!
                    { key: btnShift, action: 'contextToggle:254:1:0' },
                ]}
                selected={selected}
                contextValue={0}
            />
        </>
    );
}
