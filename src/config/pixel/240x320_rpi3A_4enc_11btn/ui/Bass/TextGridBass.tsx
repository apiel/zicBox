import * as React from '@/libs/react';

import { Rect } from '@/libs/components/Rect';
import { Bass } from '../components/Common';
import { TextGridSel } from '../components/TextGridSel';
import { Title } from '../components/Title';
import { btn1, btn2, btn3, btn4, btn5, btn6, btn7, btn8, btnDown, btnShift, btnUp, ColorTrack2 } from '../constants';

export function TextGridBass({ selected, viewName }: { selected: string; viewName: string }) {
    return (
        <>
            <Title title={Bass} />
            <Rect position={[70, 28, 6, 6]} color={ColorTrack2} />
            <TextGridSel
                selectedBackground={ColorTrack2}
                rows={['Main Fx Env Wave', 'Seq. &empty &empty &empty']}
                keys={[
                    { key: btn1, action: 'setView:Bass' },
                    { key: btn2, action: 'setView:BassFx' },
                    { key: btn3, action: 'setView:BassEnv' },
                    { key: btn4, action: 'setView:BassWaveform' },

                    { key: btn5, action: 'setView:BassSeq' },
                    { key: btn6, action: 'noteOn:Bass:60' },
                    { key: btn7, action: 'noteOn:Bass:60' },
                    { key: btn8, action: 'noteOn:Bass:60' },

                    { key: btnUp, action: 'noteOn:Bass:60' }, // when not used, let's play noteOn...
                    { key: btnDown, action: 'noteOn:Bass:60' }, // <--- this should be the default noteOn!
                    { key: btnShift, action: 'contextToggle:254:1:0' },
                ]}
                selected={selected}
                contextValue={0}
            />
        </>
    );
}
