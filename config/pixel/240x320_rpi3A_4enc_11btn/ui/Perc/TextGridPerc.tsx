import * as React from '@/libs/react';

import { Rect } from '@/libs/nativeComponents/Rect';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { Perc } from '../components/Common';
import { SideInfo } from '../components/SideInfo';
import { Title } from '../components/Title';
import {
    btn1,
    btn2,
    btn3,
    btn4,
    btn5,
    btn6,
    btn7,
    btn8,
    btnDown,
    btnShift,
    btnUp,
    ColorTrack6,
    KeyInfoPosition,
} from '../constants';

export function TextGridPerc({ selected, viewName }: { selected: string; viewName: string }) {
    return (
        <>
            <Title title={Perc} />
            <Rect bounds={[70, 28, 6, 6]} color={ColorTrack6} />
            <TextGrid
                bounds={KeyInfoPosition}
                selectedBackground={ColorTrack6}
                rows={['Main Fx Env Noise1', 'Seq. Seq2 Mod. Noise2']}
                keys={[
                    { key: btn1, action: 'setView:Perc' },
                    { key: btn2, action: 'setView:PercFx' },
                    { key: btn3, action: 'setView:PercEnv' },
                    { key: btn4, action: 'setView:PercNoise' },

                    { key: btn5, action: 'setView:PercSeq' },
                    { key: btn6, action: 'setView:PercSeq2' },
                    { key: btn7, action: 'setView:PercMod' },
                    { key: btn8, action: 'setView:PercNoise2' },

                    { key: btnUp, action: 'noteOn:Perc:60' }, // when not used, let's play noteOn...
                    { key: btnDown, action: 'noteOn:Perc:60' }, // <--- this should be the default noteOn!
                    { key: btnShift, action: 'contextToggle:254:1:0' },
                ]}
                selected={selected}
                contextValue={0}
            />
            <SideInfo up="*" down="&icon::musicNote::pixelated" ctxValue={0} />
        </>
    );
}
