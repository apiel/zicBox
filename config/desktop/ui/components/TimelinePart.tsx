import * as React from '@/libs/react';

import {
    track1Timeline,
    track2Timeline,
    track3Timeline,
    track4Timeline,
    track5Timeline,
    track6Timeline,
    workspaceFolder,
} from '@/desktop/audio';
import { ResizeType } from '@/libs/nativeComponents/component';
import { Timeline } from '@/libs/nativeComponents/Timeline';
import { unshiftVisibilityContext } from '../components/ShiftLayout';
import {
    A2,
    B1,
    B2,
    B3,
    ColorTrack1,
    ColorTrack2,
    ColorTrack3,
    ColorTrack4,
    ColorTrack5,
    ColorTrack6,
    ScreenWidth,
    stepContextId,
    Track1,
    Track2,
    Track3,
    Track4,
    Track5,
    Track6,
    trackContextId,
    viewStepStartContextId,
} from '../constants';

const top = 180;
const height = 65;
const defaultSelectedTrack = Track1;

export function TimelinePart() {
    const resizeType =
        ResizeType.RESIZE_W | ResizeType.RESIZE_X | ResizeType.RESIZE_H | ResizeType.RESIZE_Y;

    const commonProps = {
        workspaceFolder: workspaceFolder,
        scrollEncoderId: 9, // Does not exist but for testing...
        resizeType,
        defaultSelectedTrack,
        trackContextId,
        stepContextId,
        viewStepStartContextId,
        trackMin: 1,
        trackMax: 6,
        keys: [
            { key: B1, action: '.clipNext:-1', multipleKeyHandler: true },
            { key: B3, action: '.clipNext:1', multipleKeyHandler: true },
        ],
    };
    return (
        <>
            <Timeline
                timelineFilename={track1Timeline}
                enginePlugin="Track1"
                clipFolder="track1"
                bounds={[0, top, ScreenWidth, height]}
                visibilityContext={[unshiftVisibilityContext]}
                clipColor={ColorTrack1}
                track={Track1}
                {...{
                    ...commonProps,
                    keys: [
                        ...commonProps.keys,
                        { key: A2, action: '.trackNext:-1' },
                        { key: B2, action: '.trackNext:1' },
                    ],
                }}
            />

            <Timeline
                timelineFilename={track2Timeline}
                enginePlugin="Track2"
                clipFolder="track2"
                bounds={[0, top + height, ScreenWidth, height]}
                visibilityContext={[unshiftVisibilityContext]}
                clipColor={ColorTrack2}
                track={Track2}
                {...commonProps}
            />

            <Timeline
                timelineFilename={track3Timeline}
                enginePlugin="Track3"
                clipFolder="track3"
                bounds={[0, top + height * 2, ScreenWidth, height]}
                visibilityContext={[unshiftVisibilityContext]}
                clipColor={ColorTrack3}
                track={Track3}
                {...commonProps}
            />

            <Timeline
                timelineFilename={track4Timeline}
                enginePlugin="Track4"
                clipFolder="track4"
                bounds={[0, top + height * 3, ScreenWidth, height]}
                visibilityContext={[unshiftVisibilityContext]}
                clipColor={ColorTrack4}
                track={Track4}
                {...commonProps}
            />

            <Timeline
                timelineFilename={track5Timeline}
                enginePlugin="Track5"
                clipFolder="track5"
                bounds={[0, top + height * 4, ScreenWidth, height]}
                visibilityContext={[unshiftVisibilityContext]}
                clipColor={ColorTrack5}
                track={Track5}
                {...commonProps}
            />

            <Timeline
                timelineFilename={track6Timeline}
                enginePlugin="Track6"
                clipFolder="track6"
                bounds={[0, top + height * 5, ScreenWidth, height]}
                visibilityContext={[unshiftVisibilityContext]}
                clipColor={ColorTrack6}
                track={Track6}
                {...commonProps}
            />
        </>
    );
}
