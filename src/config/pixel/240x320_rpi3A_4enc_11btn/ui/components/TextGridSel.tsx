import * as React from '@/libs/react';

import { HiddenValue } from '@/libs/nativeComponents/HiddenValue';
import { Keymaps } from '@/libs/nativeComponents/Keymaps';
import { VisibilityContainer } from '@/libs/nativeComponents/VisibilityContainer';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { KeyInfoPosition } from '../constants';

export function TextGridSel({
    selected,
    rows,
    keys,
    contextValue,
    selectedBackground,
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
        <VisibilityContainer bounds={KeyInfoPosition}>
            {/* <VisibilityContext index={254} condition="SHOW_WHEN" value={contextValue} /> */}
            <HiddenValue>
                <Keymaps keys={keys} />
            </HiddenValue>
            <TextGrid bounds={[0, 0, KeyInfoPosition[2], KeyInfoPosition[3]]} rows={rows} activeBgColor={selectedBackground} />
        </VisibilityContainer>
    );
}
