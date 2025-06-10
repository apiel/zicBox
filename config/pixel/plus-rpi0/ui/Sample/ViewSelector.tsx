import * as React from '@/libs/react';

import { TextGrid } from '@/libs/tsComponents/TextGrid';
import {
    btn1,
    btn2,
    btn3,
    KeyInfoPosition
} from '../constants';

export function ViewSelector({
    selected,
    viewName,
    synthName,
    color,
    pageCount,
    currentPage,
}: {
    selected?: string;
    viewName: string;
    synthName: string;
    color: string;
    pageCount?: number;
    currentPage?: number;
}) {
    return (
        <TextGrid
            bounds={KeyInfoPosition}
            selectedBackground={color}
            rows={[`Fx Wave Browse &empty &empty`]}
            keys={[
                { key: btn1, action: `setView:${synthName}Fx` },
                { key: btn2, action: `setView:${synthName}Wave` },
                { key: btn3, action: `setView:${synthName}Browse` },
            ]}
            selected={selected}
            contextValue={[0, 0, 0]}
            pageCount={pageCount}
            currentPage={currentPage}
        />
    );
}
