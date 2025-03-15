import * as React from '@/libs/react';

import { Rect } from '@/libs/nativeComponents/Rect';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { Synth } from '../components/Common';
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
    ColorTrack4,
    KeyInfoPosition,
} from '../constants';

export function TextGridSynth({ selected, viewName }: { selected: string; viewName: string }) {
    return (
        <>
            <Title title={Synth} />
            <Rect bounds={[70, 28, 6, 6]} color={ColorTrack4} />
            <TextGrid
                bounds={KeyInfoPosition}
                selectedBackground={ColorTrack4}
                rows={['Main Filter Env Mod.', 'Seq. Fx Lfo Mod']}
                keys={[
                    { key: btn1, action: 'setView:Synth' },
                    { key: btn2, action: 'setView:SynthFilter' },
                    {
                        key: btn3,
                        action:
                            viewName === 'SynthEnv1'
                                ? 'setView:SynthEnv2'
                                : 'setView:SynthEnv1',
                    },
                    {
                        key: btn4,
                        action:
                            viewName === 'SynthEnv2'
                                ? 'setView:SynthEnv1'
                                : 'setView:SynthEnv2',
                    },

                    { key: btn5, action: 'setView:SynthSeq' },
                    { key: btn6, action: 'setView:SynthFx' },
                    {
                        key: btn7,
                        action:
                            viewName === 'SynthLfo1'
                                ? 'setView:SynthLfo2'
                                : 'setView:SynthLfo1',
                    },
                    {
                        key: btn8,
                        action:
                            viewName === 'SynthLfo2'
                                ? 'setView:SynthLfo1'
                                : 'setView:SynthLfo2',
                    },

                    { key: btnUp, action: 'noteOn:Synth:58' }, // when not used, let's play noteOn...
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
