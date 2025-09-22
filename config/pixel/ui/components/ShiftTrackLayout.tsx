import * as React from '@/libs/react';

import { VisibilityContext } from '@/libs/nativeComponents/component';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { Rect } from '@/libs/nativeComponents/Rect';
import { Text } from '@/libs/nativeComponents/Text';
import {
    C1,
    C2,
    C4,
    Drum4Track,
    MasterTrack,
    menuTextColor,
    ScreenHeight,
    shiftContext,
    W1_4,
    W3_4,
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
    synthName,
}: {
    content?: any;
    track: number;
    label?: string;
    synthName: string;
}) {
    return (
        <>
            <Rect
                bounds={backgroundBounds}
                color="background"
                visibilityContext={[shiftTrackVisibilityContext]}
                keys={[
                    {
                        key: C1,
                        action: `contextToggle:${shiftContext}:2:0`,
                        action2: `setView:${synthName}Seq`,
                        context: { id: shiftContext, value: 2 },
                    },
                    {
                        key: C2,
                        action: `contextToggle:${shiftContext}:2:0`,
                        action2: `setView:${synthName}Keyboard`,
                        context: { id: shiftContext, value: 2 },
                    },
                    { key: C4, action: `mute:${track}`, context: { id: shiftContext, value: 2 } },
                ]}
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

            <Text
                text="Sequencer"
                bounds={[0, ScreenHeight - 20, W1_4, 16]}
                centered={true}
                visibilityContext={[shiftTrackVisibilityContext]}
                color={menuTextColor}
            />

            <Text
                text="Keyboard"
                bounds={[W1_4, ScreenHeight - 20, W1_4, 16]}
                centered={true}
                visibilityContext={[shiftTrackVisibilityContext]}
                color={menuTextColor}
            />

            {/* <Text
                text="Reload All"
                bounds={[W2_4, ScreenHeight - 20, W1_4, 16]}
                centered={true}
                visibilityContext={[shiftTrackVisibilityContext]}
                color={menuTextColor}
            /> */}

            <Text
                text="Mute"
                bounds={[W3_4, ScreenHeight - 20, W1_4, 16]}
                centered={true}
                visibilityContext={[shiftTrackVisibilityContext]}
                color={menuTextColor}
            />

            {/* <Rect
                bounds={backgroundBounds}
                color="background"
                visibilityContext={[unshiftVisibilityContext]}
            /> */}
        </>
    );
}
