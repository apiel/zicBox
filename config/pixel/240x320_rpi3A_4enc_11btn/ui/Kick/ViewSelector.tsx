import * as React from '@/libs/react';

import { Rect } from '@/libs/nativeComponents/Rect';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { lighten } from '@/libs/ui';
import { Kick } from '../components/Common';
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
    color,
    synthName,
    viewName,
    hideTitle,
    pageCount,
    currentPage,
}: {
    selected: string;
    color: string;
    synthName: string;
    viewName: string;
    hideTitle?: boolean;
    pageCount?: number;
    currentPage?: number;
}) {
    return (
        <>
            {!hideTitle && <Title title={Kick} />}
            {!hideTitle && <Rect bounds={[70, 28, 6, 6]} color={color} />}
            <TextGrid
                bounds={KeyInfoPosition}
                selectedBackground={color}
                shiftedTextColor={lighten(color, 0.5)}
                pageCount={pageCount}
                currentPage={currentPage}
                rows={['Main Fx Wave Freq', `Seq. Amp Layer2 &icon::musicNote::pixelated`]}
                keys={[
                    { key: btn1, action: `setView:${synthName}` },
                    {
                        key: btn2,
                        action:
                            viewName === `${synthName}Fx`
                                ? `setView:${synthName}Fx2`
                                : `setView:${synthName}Fx`,
                    },
                    { key: btn3, action: `setView:${synthName}Waveform` },
                    { key: btn4, action: `setView:${synthName}Frequency` },
                    ...(viewName === `${synthName}Sequencer`
                        ? []
                        : [{ key: btn5, action: `setView:${synthName}Sequencer` }]),
                    { key: btn6, action: `setView:${synthName}Amplitude` },
                    {
                        key: btn7,
                        action:
                            viewName === `${synthName}Layer2`
                                ? `setView:${synthName}Layer2_2`
                                : `setView:${synthName}Layer2`,
                    },
                    ...(selected === 'Seq.'
                        ? []
                        : [{ key: btn8, action: `noteOn:${synthName}:60` }]),

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
