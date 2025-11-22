import * as React from '@/libs/react';

import { track1Timeline, track2Timeline, track3Timeline, track4Timeline, track5Timeline, track6Timeline, workspaceFolder } from '@/desktop/audio';
import { ResizeType } from '@/libs/nativeComponents/component';
import { Timeline } from '@/libs/nativeComponents/Timeline';
import { unshiftVisibilityContext } from '../components/ShiftLayout';
import { ColorTrack1, ColorTrack2, ColorTrack3, ColorTrack4, ColorTrack5, ColorTrack6, ScreenWidth } from '../constants';

const top = 180;
const height = 65;

export function TimelinePart() {
    const resizeType = ResizeType.RESIZE_W | ResizeType.RESIZE_X | ResizeType.RESIZE_H | ResizeType.RESIZE_Y;
    return (
        <>
            <Timeline
                workspaceFolder={workspaceFolder}
                timelineFilename={track1Timeline}
                enginePlugin="Track1"
                clipFolder="track1"
                scrollEncoderId={9} // Does not exist but for testing...
                bounds={[0, top, ScreenWidth, height]}
                visibilityContext={[unshiftVisibilityContext]}
                resizeType={resizeType}
                clipColor={ColorTrack1}
            />

            <Timeline
                workspaceFolder={workspaceFolder}
                timelineFilename={track2Timeline}
                enginePlugin="Track2"
                clipFolder="track2"
                scrollEncoderId={9} // Does not exist but for testing...
                bounds={[0, top + height, ScreenWidth, height]}
                visibilityContext={[unshiftVisibilityContext]}
                resizeType={resizeType}
                clipColor={ColorTrack2}
            />

            <Timeline
                workspaceFolder={workspaceFolder}
                timelineFilename={track3Timeline}
                enginePlugin="Track3"
                clipFolder="track3"
                scrollEncoderId={9} // Does not exist but for testing...
                bounds={[0, top + height * 2, ScreenWidth, height]}
                visibilityContext={[unshiftVisibilityContext]}
                resizeType={resizeType}
                clipColor={ColorTrack3}
            />

            <Timeline
                workspaceFolder={workspaceFolder}
                timelineFilename={track4Timeline}
                enginePlugin="Track4"
                clipFolder="track4"
                scrollEncoderId={9} // Does not exist but for testing...
                bounds={[0, top + height * 3, ScreenWidth, height]}
                visibilityContext={[unshiftVisibilityContext]}
                resizeType={resizeType}
                clipColor={ColorTrack4}
            />

            <Timeline
                workspaceFolder={workspaceFolder}
                timelineFilename={track5Timeline}
                enginePlugin="Track5"
                clipFolder="track5"
                scrollEncoderId={9} // Does not exist but for testing...
                bounds={[0, top + height * 4, ScreenWidth, height]}
                visibilityContext={[unshiftVisibilityContext]}
                resizeType={resizeType}
                clipColor={ColorTrack5}
            />

            <Timeline
                workspaceFolder={workspaceFolder}
                timelineFilename={track6Timeline}
                enginePlugin="Track6"
                clipFolder="track6"
                scrollEncoderId={9} // Does not exist but for testing...
                bounds={[0, top + height * 5, ScreenWidth, height]}
                visibilityContext={[unshiftVisibilityContext]}
                resizeType={resizeType}
                clipColor={ColorTrack6}
            />
        </>
    );
}
