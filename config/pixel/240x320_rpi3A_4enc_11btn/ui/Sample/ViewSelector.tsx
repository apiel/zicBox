import * as React from '@/libs/react';

import { Rect } from '@/libs/nativeComponents/Rect';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
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
            {!hideTitle && <Rect bounds={[55, 28, 6, 6]} color={color} />}
            <TextGrid
                bounds={KeyInfoPosition}
                selectedBackground={color}
                rows={[
                    'Main Fx Wave Browse',
                    `Seq. &icon::musicNote::pixelated &icon::musicNote::pixelated ${
                        selected === 'Seq.' ? '___' : '&icon::musicNote::pixelated'
                    }`,
                ]}
                keys={[
                    { key: btn1, action: `setView:${synthName}` },
                    { key: btn2, action: `setView:${synthName}Fx` },
                    {
                        key: btn3,
                        action:
                            viewName === `${synthName}Wave`
                                ? `setView:${synthName}Browse`
                                : `setView:${synthName}Wave`,
                    },
                    {
                        key: btn4,
                        action:
                            viewName === `${synthName}Browse`
                                ? `setView:${synthName}Wave`
                                : `setView:${synthName}Browse`,
                    },

                    { key: btn5, action: `setView:${synthName}Seq` },
                    { key: btn6, action: `noteOn:${synthName}:72` },
                    { key: btn7, action: `noteOn:${synthName}:48` },
                    ...(selected === 'Seq.' ? [] : [{ key: btn8, action: `noteOn:${synthName}:60` }]),

                    { key: btnUp, action: 'contextToggle:253:1:0' }, // when not used, let's play noteOn...
                    { key: btnDown, action: 'contextToggleOnRelease:252:1:0' },
                    { key: btnShift, action: 'contextToggle:254:1:0' },
                ]}
                selected={selected}
                contextValue={[0, 0, 0]}
            />
        </>
    );
}
