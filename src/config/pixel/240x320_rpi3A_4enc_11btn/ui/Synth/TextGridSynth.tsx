import * as React from '@/libs/react';

import { Rect } from '@/libs/components/Rect';
import { Synth } from '../components/Common';
import { SideInfo } from '../components/SideInfo';
import { TextGridSel } from '../components/TextGridSel';
import { Title } from '../components/Title';
import {
    btn1,
    btn3,
    btn5,
    btn6,
    btn7,
    btn8,
    btnDown,
    btnShift,
    btnUp,
    ColorTrack4
} from '../constants';

export function TextGridSynth({ selected, viewName }: { selected: string; viewName: string }) {
    return (
        <>
            <Title title={Synth} />
            <Rect position={[70, 28, 6, 6]} color={ColorTrack4} />
            <TextGridSel
                selectedBackground={ColorTrack4}
                rows={['Main Fx Env1/2 &empty', 'Seq. &empty &empty &empty']}
                keys={[
                    { key: btn1, action: 'setView:Synth' },
                    // { key: btn2, action: 'setView:SynthFx' },
                    { key: btn3, action: viewName === 'SynthEnv1' ? 'setView:SynthEnv2' : 'setView:SynthEnv1' },
                    // { key: btn4, action: 'setView:BassWaveform' },

                    { key: btn5, action: 'setView:SynthSeq' },
                    { key: btn6, action: 'noteOn:Synth:60' },
                    { key: btn7, action: 'noteOn:Synth:60' },
                    { key: btn8, action: 'noteOn:Synth:60' },

                    { key: btnUp, action: 'noteOn:Synth:60' }, // when not used, let's play noteOn...
                    { key: btnDown, action: 'noteOn:Synth:60' }, // <--- this should be the default noteOn!
                    { key: btnShift, action: 'contextToggle:254:1:0' },
                ]}
                selected={selected}
                contextValue={0}
            />
            <SideInfo up="*" down="&icon::musicNote::pixelated" ctxValue={0} />
        </>
    );
}
