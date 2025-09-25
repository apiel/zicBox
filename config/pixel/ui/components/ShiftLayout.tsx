import * as React from '@/libs/react';

import { ShiftMainLayout } from './ShiftMainLayout';
import { ShiftTrackLayout } from './ShiftTrackLayout';

export { notUnshiftVisibilityContext, shiftVisibilityContext, unshiftVisibilityContext } from './ShiftMainLayout';
export { shiftTrackVisibilityContext } from './ShiftTrackLayout';

export function ShiftLayout({
    track,
    label,
    synthName,
}: {
    track: number;
    label?: string;
    synthName: string;
}) {
    return (
        <>
            {/* ShiftTrackLayout must be before ShiftMainLayout */}
            <ShiftTrackLayout track={track} label={label} synthName={synthName} />
            <ShiftMainLayout synthName={synthName} track={track} label={label} />
        </>
    );
}
