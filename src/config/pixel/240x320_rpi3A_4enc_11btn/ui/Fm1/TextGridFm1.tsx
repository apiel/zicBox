import * as React from '@/libs/react';

import { Rect } from '@/libs/components/Rect';
import { Bass } from '../components/Common';
import { SideInfo } from '../components/SideInfo';
import { TextGridSel } from '../components/TextGridSel';
import { Title } from '../components/Title';
import {
    btn1,
    btn2,
    btn3,
    btn5,
    btn6,
    btn7,
    btn8,
    btnDown,
    btnShift,
    btnUp,
    ColorTrack2,
} from '../constants';

export function TextGridFm1({ selected, viewName }: { selected: string; viewName: string }) {
    return (
        <>
            <Title title={Bass} />
            <Rect position={[70, 28, 6, 6]} color={ColorTrack2} />
            <TextGridSel
                selectedBackground={ColorTrack2}
                rows={['Main Fx Env &empty', 'Seq. &empty &empty &empty']}
                keys={[
                    { key: btn1, action: 'setView:Fm1' },
                    { key: btn2, action: 'setView:Fm1Fx' },
                    { key: btn3, action: 'setView:Fm1Env' },
                    // { key: btn4, action: 'setView:BassWaveform' },

                    { key: btn5, action: 'setView:Fm1Seq' },
                    { key: btn6, action: 'noteOn:FmDrum:60' },
                    { key: btn7, action: 'noteOn:FmDrum:60' },
                    { key: btn8, action: 'noteOn:FmDrum:60' },

                    { key: btnUp, action: 'noteOn:FmDrum:60' }, // when not used, let's play noteOn...
                    { key: btnDown, action: 'noteOn:FmDrum:60' }, // <--- this should be the default noteOn!
                    { key: btnShift, action: 'contextToggle:254:1:0' },
                ]}
                selected={selected}
                contextValue={0}
            />
            <SideInfo up="*" down="&icon::musicNote::pixelated" ctxValue={0} />
        </>
    );
}
