import * as React from '@/libs/react';

import { VisibilityContext } from '@/libs/nativeComponents/component';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { Rect } from '@/libs/nativeComponents/Rect';
import {
    Drum4Track,
    MasterTrack,
    shiftContext
} from '../constants';
import { backgroundBounds, enc1, enc3 } from '../constantsValue';

export const shiftTrackVisibilityContext: VisibilityContext = {
    condition: 'SHOW_WHEN',
    index: shiftContext,
    value: 2,
};

export function ShiftTrackLayout({
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
            <Rect
                bounds={backgroundBounds}
                color="background"
                visibilityContext={[shiftTrackVisibilityContext]}
            />

            <KnobValue
                audioPlugin="Mixer"
                param={`TRACK_${track}`}
                label={label}
                {...enc1}
                color="primary"
                track={MasterTrack}
                visibilityContext={[shiftTrackVisibilityContext]}
            />

            {content}

            {track <= Drum4Track && (
                <KnobValue
                    audioPlugin="Sequencer"
                    param="NOTE_REPEAT"
                    {...enc3}
                    color="tertiary"
                    track={track}
                    visibilityContext={[shiftTrackVisibilityContext]}
                />
            )}

            {/* <KnobValue
                audioPlugin="TrackFx"
                param="VOLUME"
                label="Master Vol."
                {...enc4}
                color="tertiary"
                track={MasterTrack}
                visibilityContext={[shiftTrackVisibilityContext]}
            /> */}

            {/* <Text
                text="&icon::play::filled"
                bounds={[0, ScreenHeight - 18, W1_4, 16]}
                centered={true}
                visibilityContext={[shiftTrackVisibilityContext]}
                color={menuTextColor}
            /> */}

            {/* <Text
                text="Menu"
                bounds={[W1_4, ScreenHeight - 20, W1_4, 16]}
                centered={true}
                visibilityContext={[shiftTrackVisibilityContext]}
                color={menuTextColor}
            /> */}

            {/* <Text
                text="Reload All"
                bounds={[W2_4, ScreenHeight - 20, W1_4, 16]}
                centered={true}
                visibilityContext={[shiftTrackVisibilityContext]}
                color={menuTextColor}
            /> */}

            {/* <Text
                text="Shift"
                bounds={[W3_4, ScreenHeight - 20, W1_4, 16]}
                centered={true}
                visibilityContext={[shiftTrackVisibilityContext]}
                color={rgb(80, 80, 80)}
            /> */}

            {/* <Rect
                bounds={backgroundBounds}
                color="background"
                visibilityContext={[unshiftVisibilityContext]}
            /> */}
        </>
    );
}
