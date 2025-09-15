import * as React from '@/libs/react';

import { ShiftMainLayout } from './ShiftMainLayout';
import { ShiftTrackLayout } from './ShiftTrackLayout';

export { notUnshiftVisibilityContext, shiftVisibilityContext, unshiftVisibilityContext } from './ShiftMainLayout';
export { shiftTrackVisibilityContext } from './ShiftTrackLayout';

export function ShiftLayout({
    content,
    track,
    label,
    synthName,
}: {
    content?: any;
    track: number;
    label?: string;
    synthName: string;
}) {
    return (
        <>
            {/* ShiftTrackLayout must be before ShiftMainLayout */}
            <ShiftTrackLayout content={content} track={track} label={label} synthName={synthName} />
            <ShiftMainLayout content={content} track={track} label={label} />
        </>
    );
}
