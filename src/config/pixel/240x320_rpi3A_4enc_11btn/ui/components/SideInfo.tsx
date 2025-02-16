import * as React from '@/libs/react';

import { TextGrid } from '@/libs/tsComponents/TextGrid';
import { SideColor, SideKeyInfoPosition } from '../constants';

export function SideInfo({ up, down, ctxValue }: { up: string; down: string; ctxValue: number }) {
    return (
        <TextGrid
            bounds={SideKeyInfoPosition}
            rows={[up, down]}
            bgColor={SideColor}
            visibilityCondition="SHOW_WHEN"
            visibilityContext={254}
            visibilityValue={ctxValue}
        />
    );
}
