import * as React from '@/libs/react';

import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { lighten } from '@/libs/ui';
import { btn1, btn2, btn3, btn4, btn5, KeyInfoPosition } from '../constants';

export function ViewSelector({
    selected,
    color,
    synthName,
    viewName,
    hideTitle,
    pageCount,
    currentPage,
}: {
    selected?: string;
    color: string;
    synthName: string;
    viewName: string;
    hideTitle?: boolean;
    pageCount?: number;
    currentPage?: number;
}) {
    return (
        <TextGrid
            bounds={KeyInfoPosition}
            selectedBackground={color}
            shiftedTextColor={lighten(color, 0.5)}
            pageCount={pageCount}
            currentPage={currentPage}
            rows={['Fx Wave Freq Amp Layer2']}
            keys={[
                {
                    key: btn1,
                    action:
                        viewName === `${synthName}Fx`
                            ? `setView:${synthName}Fx:page2`
                            : `setView:${synthName}Fx`,
                },
                { key: btn2, action: `setView:${synthName}Waveform` },
                { key: btn3, action: `setView:${synthName}Frequency` },
                { key: btn4, action: `setView:${synthName}Amplitude` },
                {
                    key: btn5,
                    action:
                        viewName === `${synthName}Layer2`
                            ? `setView:${synthName}Layer2:page2`
                            : `setView:${synthName}Layer2`,
                },
            ]}
            selected={selected}
            contextValue={[0, 0, 0]}
        />
    );
}
