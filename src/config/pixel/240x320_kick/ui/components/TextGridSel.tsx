import * as React from '@/libs/react';

import { Keymaps } from '@/libs/components/Keymaps';
import { TextGrid } from '@/libs/components/TextGrid';
import { VisibilityContext } from '@/libs/components/VisibilityContext';
import { KeyInfoPosition } from '@/pixel/240x320_kick/ui/constants';

export function TextGridSel({
    selected,
    items,
    keys,
    contextValue,
}: {
    selected: number;
    items: [string, string, string, string, string, string];
    keys: { key: string; action: string; action2?: string }[];
    contextValue: number;
}) {
    if (selected >= 0) {
        items[selected] = `!${items[selected]}`;
    }
    return (
        <TextGrid
            position={KeyInfoPosition}
            rows={[`${items[0]} ${items[1]} ${items[2]}`, `${items[3]} ${items[4]} ${items[5]}`]}
        >
            <VisibilityContext index={254} condition="SHOW_WHEN" value={contextValue} />
            <Keymaps keys={keys} />
        </TextGrid>
    );
}