import * as React from '@/libs/react';

import { Keymaps } from '@/libs/components/Keymaps';
import { TextGrid } from '@/libs/components/TextGrid';
import { VisibilityContext } from '@/libs/components/VisibilityContext';
import { KeyInfoPosition } from '@/pixel/240x320_kick/ui/constants';

export function TextGridSel({
    selected,
    rows,
    keys,
    contextValue,
    selectedBackground
}: {
    selected?: string;
    rows: string[];
    keys: { key: string; action: string; action2?: string }[];
    contextValue: number;
    selectedBackground?: string;
}) {
    if (selected) {
        for (let i = 0; i < rows.length; i++) {
            rows[i] = rows[i].replace(selected, `!${selected}`);
        }
    }
    return (
        <TextGrid
            position={KeyInfoPosition}
            rows={rows}
            ITEM_BACKGROUND={selectedBackground}
        >
            <VisibilityContext index={254} condition="SHOW_WHEN" value={contextValue} />
            <Keymaps keys={keys} />
        </TextGrid>
    );
}
