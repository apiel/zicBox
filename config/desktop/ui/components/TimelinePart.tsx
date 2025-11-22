import * as React from '@/libs/react';

import { track1Timeline, workspaceFolder } from '@/desktop/audio';
import { ResizeType } from '@/libs/nativeComponents/component';
import { Timeline } from '@/libs/nativeComponents/Timeline';
import { unshiftVisibilityContext } from '../components/ShiftLayout';
import { ColorTrack1, ScreenWidth } from '../constants';

export function TimelinePart() {
    const resizeType = ResizeType.RESIZE_W | ResizeType.RESIZE_X;
    return (
        <>
            <Timeline
                workspaceFolder={workspaceFolder}
                timelineFilename={track1Timeline}
                enginePlugin="Track1"
                clipFolder="track1"
                scrollEncoderId={9} // Does not exist but for testing...
                bounds={[0, 180, ScreenWidth, 100]}
                visibilityContext={[unshiftVisibilityContext]}
                resizeType={resizeType}
                clipColor={ColorTrack1}
            />
        </>
    );
}
