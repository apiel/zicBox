import * as React from '@/libs/react';

import { track1Timeline, track2Timeline, track3Timeline, track4Timeline, track5Timeline, track6Timeline, workspaceFolder } from '@/desktop/audio';
import { ResizeType } from '@/libs/nativeComponents/component';
import { Timeline } from '@/libs/nativeComponents/Timeline';
import { unshiftVisibilityContext } from '../components/ShiftLayout';
import { B1, B3, ColorTrack1, ColorTrack2, ColorTrack3, ColorTrack4, ColorTrack5, ColorTrack6, ScreenWidth, Track1, Track2, Track3, Track4, Track5, Track6 } from '../constants';

const top = 180;
const height = 65;
const defaultSelectedTrack = Track1;

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
                track={Track1}
                defaultSelectedTrack={defaultSelectedTrack}
                keys={[
                    { key: B1, action: '.clipNext:-1' },
                    { key: B3, action: '.clipNext:1' },
                ]}
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
                track={Track2}
                defaultSelectedTrack={defaultSelectedTrack}
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
                track={Track3}
                defaultSelectedTrack={defaultSelectedTrack}
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
                track={Track4}
                defaultSelectedTrack={defaultSelectedTrack}
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
                track={Track5}
                defaultSelectedTrack={defaultSelectedTrack}
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
                track={Track6}
                defaultSelectedTrack={defaultSelectedTrack}
            />
        </>
    );
}
