import * as React from '@/libs/react';

import { TextGrid } from '@/libs/nativeComponents/TextGrid';
import { VisibilityContext } from '@/libs/nativeComponents/VisibilityContext';
import {
    SideColor,
    SideKeyInfoPosition
} from '../constants';

export function SideInfo({ up, down, ctxValue }: { up: string; down: string; ctxValue: number }) {
    return (
        <TextGrid
            bounds={SideKeyInfoPosition}
            rows={[up, down]}
            BACKGROUND_COLOR={SideColor}
        >
            <VisibilityContext index={254} condition="SHOW_WHEN" value={ctxValue} />
        </TextGrid>
    );
}
