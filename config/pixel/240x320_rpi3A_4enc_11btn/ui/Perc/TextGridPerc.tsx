import * as React from '@/libs/react';

import { Rect } from '@/libs/nativeComponents/Rect';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { Perc } from '../components/Common';
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
    ColorTrack5,
    KeyInfoPosition,
} from '../constants';

export function TextGridPerc({ selected, viewName }: { selected: string; viewName: string }) {
    return (
        <>
            <Title title={Perc} />
            <Rect bounds={[70, 28, 6, 6]} color={ColorTrack5} />
            <TextGrid
                bounds={KeyInfoPosition}
                selectedBackground={ColorTrack5}
                rows={[
                    'Main Fx Noise1 Noise2',
                    `Seq. Mod. &icon::musicNote::pixelated ${
                        selected === 'Seq.' ? '___' : '&icon::musicNote::pixelated'
                    }`,
                ]}
                keys={[
                    { key: btn1, action: 'setView:Perc' },
                    { key: btn2, action: 'setView:PercFx' },
                    { key: btn3, action: 'setView:PercNoise' },
                    { key: btn4, action: 'setView:PercNoise2' },

                    { key: btn5, action: 'setView:PercSeq' },
                    { key: btn6, action: 'setView:PercMod' },
                    { key: btn7, action: 'noteOn:Perc:48' },
                    ...(selected === 'Seq.' ? [] : [{ key: btn8, action: `noteOn:Perc:60` }]),

                    { key: btnUp, action: 'contextToggle:253:1:0' },
                    { key: btnDown, action: 'contextToggleOnRelease:252:1:0' },
                    { key: btnShift, action: 'contextToggle:254:1:0' },
                ]}
                selected={selected}
                contextValue={[0, 0, 0]}
            />
        </>
    );
}
