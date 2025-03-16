import * as React from '@/libs/react';

import { Rect } from '@/libs/nativeComponents/Rect';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
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
    KeyInfoPosition,
} from '../constants';

export function ViewSelector({
    selected,
    viewName,
    synthName,
    color,
    hideTitle,
}: {
    selected: string;
    viewName: string;
    synthName: string;
    color: string;
    hideTitle?: boolean;
}) {
    return (
        <>
            {!hideTitle && <Title title={synthName} />}
            {!hideTitle && <Rect bounds={[60, 28, 6, 6]} color={color} />}
            <TextGrid
                bounds={KeyInfoPosition}
                selectedBackground={color}
                rows={['Main Env/Fx Lfo Wave', 'Seq. Mod. Mod &icon::musicNote::pixelated']}
                keys={[
                    { key: btn1, action: `setView:${synthName}` },
                    {
                        key: btn2,
                        action:
                            viewName === `${synthName}Env1`
                                ? `setView:${synthName}Env2`
                                : `setView:${synthName}Env1`,
                    },
                    {
                        key: btn3,
                        action:
                            viewName === `${synthName}Lfo1`
                                ? `setView:${synthName}Lfo2`
                                : `setView:${synthName}Lfo1`,
                    },
                    { key: btn4, action: `setView:${synthName}Wave` },

                    { key: btn5, action: `setView:${synthName}Seq` },
                    {
                        key: btn6,
                        action:
                            viewName === `${synthName}Env2`
                                ? `setView:${synthName}Env1`
                                : `setView:${synthName}Env2`,
                    },
                    {
                        key: btn7,
                        action:
                            viewName === `${synthName}Lfo2`
                                ? `setView:${synthName}Lfo1`
                                : `setView:${synthName}Lfo2`,
                    },
                    { key: btn8, action: `noteOn:${synthName}:60` },

                    { key: btnUp, action: `noteOn:${synthName}:58` }, // when not used, let's play noteOn...
                    { key: btnDown, action: `noteOn:${synthName}:60` }, // <--- this should be the default noteOn!
                    { key: btnShift, action: 'contextToggle:254:1:0' },
                ]}
                selected={selected}
                contextValue={0}
            />
            <SideInfo up="*" down="&icon::musicNote::pixelated" ctxValue={0} />
        </>
    );
}
