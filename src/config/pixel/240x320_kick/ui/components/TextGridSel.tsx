import * as React from '@/libs/react';

import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { KeyInfoPosition } from '@/pixel/240x320_kick/ui/constants';

export function TextGridSel({
    selected,
    items,
    keys,
    contextValue,
    activeBgColor
}: {
    selected: number;
    items: [string, string, string, string, string, string];
    keys: { key: string; action: string; action2?: string }[];
    contextValue: number;
    activeBgColor?: string;
}) {
    if (selected >= 0) {
        items[selected] = `!${items[selected]}`;
    }
    return (
        <TextGrid
            bounds={KeyInfoPosition}
            rows={[`${items[0]} ${items[1]} ${items[2]}`, `${items[3]} ${items[4]} ${items[5]}`]}
            selectedBackground={activeBgColor}
            keys={keys}
            contextValue={contextValue}
        />
    );
}
