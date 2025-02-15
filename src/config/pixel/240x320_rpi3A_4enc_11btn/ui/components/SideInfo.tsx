import * as React from '@/libs/react';

import { VisibilityContext } from '@/libs/nativeComponents/VisibilityContext';
import { TextGrid } from '@/libs/tsComponents/TextGrid';
import {
    SideColor,
    SideKeyInfoPosition
} from '../constants';

export function SideInfo({ up, down, ctxValue }: { up: string; down: string; ctxValue: number }) {
    return (
        <TextGrid
            bounds={SideKeyInfoPosition}
            rows={[up, down]}
            bgColor={SideColor}
        >
            <VisibilityContext index={254} condition="SHOW_WHEN" value={ctxValue} />
        </TextGrid>
    );
}
