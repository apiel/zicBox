import * as React from '@/libs/react';

import { ShiftMainLayout } from './ShiftMainLayout';
import { ShiftTrackLayout } from './ShiftTrackLayout';

export { shiftVisibilityContext, unshiftVisibilityContext } from './ShiftMainLayout';
export { shiftTrackVisibilityContext } from './ShiftTrackLayout';

export function ShiftLayout({
    content,
    track,
    label,
}: {
    content?: any;
    track: number;
    label?: string;
}) {
    return (
        <>
            {/* ShiftTrackLayout must be before ShiftMainLayout */}
            <ShiftTrackLayout content={content} track={track} label={label} />
            <ShiftMainLayout content={content} track={track} label={label} />
        </>
    );
}
