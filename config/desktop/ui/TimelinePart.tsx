import * as React from '@/libs/react';

import { getClipFolder, getSynthAlias, workspaceFolder } from '@/desktop/audio';
import { ResizeType } from '@/libs/nativeComponents/component';
import { Timeline } from '@/libs/nativeComponents/Timeline';
import { TimelineLoop } from '@/libs/nativeComponents/TimelineLoop';
import {
    A2,
    A3,
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
    unshiftVisibilityContext,
    viewStepStartContextId,
} from './constants';
import { encEmini, encFmini } from './constantsValue';

// const top = 180;
const top = 10;
const height = 65;
const defaultSelectedTrack = Track1;

export function TimelinePart() {
    // const resizeType = ResizeType.RESIZE_W | ResizeType.RESIZE_X | ResizeType.RESIZE_H | ResizeType.RESIZE_Y;
    const resizeType = ResizeType.RESIZE_W | ResizeType.RESIZE_H | ResizeType.RESIZE_Y;

    const commonProps = {
        workspaceFolder: workspaceFolder,
        scrollEncoderId: encEmini.encoderId,
        moveClipEncoderId: encFmini.encoderId,
        resizeType,
        defaultSelectedTrack,
        timelinePlugin: 'TimelineSequencer',
        trackContextId,
        stepContextId,
        viewStepStartContextId,
        trackMin: 1,
        trackMax: 6,
        keys: [
            { key: B1, action: '.clipNext:-1', multipleKeyHandler: true },
            { key: B3, action: '.clipNext:1', multipleKeyHandler: true },
            { key: A3, action: '.moveClip', multipleKeyHandler: true },
        ],
    };
    return (
        <>
            <TimelineLoop
                timelinePlugin="Tempo"
                viewStepStartContextId={viewStepStartContextId}
                bounds={[0, 0, ScreenWidth, 9]}
                resizeType={resizeType}
            />
            <Timeline
                enginePlugin={getSynthAlias(Track1)}
                clipFolder={getClipFolder(Track1)}
                bounds={[0, top, ScreenWidth, height - 1]}
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
                enginePlugin={getSynthAlias(Track2)}
                clipFolder={getClipFolder(Track2)}
                bounds={[0, top + height, ScreenWidth, height - 1]}
                visibilityContext={[unshiftVisibilityContext]}
                clipColor={ColorTrack2}
                track={Track2}
                {...commonProps}
            />

            <Timeline
                enginePlugin={getSynthAlias(Track3)}
                clipFolder={getClipFolder(Track3)}
                bounds={[0, top + height * 2, ScreenWidth, height - 1]}
                visibilityContext={[unshiftVisibilityContext]}
                clipColor={ColorTrack3}
                track={Track3}
                {...commonProps}
            />

            <Timeline
                enginePlugin={getSynthAlias(Track4)}
                clipFolder={getClipFolder(Track4)}
                bounds={[0, top + height * 3, ScreenWidth, height - 1]}
                visibilityContext={[unshiftVisibilityContext]}
                clipColor={ColorTrack4}
                track={Track4}
                {...commonProps}
            />

            <Timeline
                enginePlugin={getSynthAlias(Track5)}
                clipFolder={getClipFolder(Track5)}
                bounds={[0, top + height * 4, ScreenWidth, height - 1]}
                visibilityContext={[unshiftVisibilityContext]}
                clipColor={ColorTrack5}
                track={Track5}
                {...commonProps}
            />

            <Timeline
                enginePlugin={getSynthAlias(Track6)}
                clipFolder={getClipFolder(Track6)}
                bounds={[0, top + height * 5, ScreenWidth, height - 1]}
                visibilityContext={[unshiftVisibilityContext]}
                clipColor={ColorTrack6}
                track={Track6}
                {...commonProps}
            />
        </>
    );
}
