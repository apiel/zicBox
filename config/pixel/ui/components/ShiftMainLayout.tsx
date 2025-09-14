import * as React from '@/libs/react';

import { VisibilityContext } from '@/libs/nativeComponents/component';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { Rect } from '@/libs/nativeComponents/Rect';
import { Text } from '@/libs/nativeComponents/Text';
import { rgb } from '@/libs/ui';
import {
    Drum4Track,
    MasterTrack,
    menuTextColor,
    ScreenHeight,
    shiftContext,
    W1_4,
    W2_4,
    W3_4,
} from '../constants';
import { backgroundBounds, enc1, enc3, enc4 } from '../constantsValue';

export const unshiftVisibilityContext: VisibilityContext = {
    condition: 'SHOW_WHEN',
    index: shiftContext,
    value: 0,
};

export const shiftVisibilityContext: VisibilityContext = {
    condition: 'SHOW_WHEN',
    index: shiftContext,
    value: 1,
};

export const notUnshiftVisibilityContext: VisibilityContext = {
    condition: 'SHOW_WHEN_NOT',
    index: shiftContext,
    value: 0,
};

export function ShiftMainLayout({
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
                visibilityContext={[shiftVisibilityContext]}
            />

            <KnobValue
                audioPlugin="Mixer"
                param={`TRACK_${track}`}
                label={label}
                {...enc1}
                color="primary"
                track={MasterTrack}
                visibilityContext={[shiftVisibilityContext]}
            />

            {content}

            {track <= Drum4Track && (
                <KnobValue
                    audioPlugin="Sequencer"
                    param="NOTE_REPEAT"
                    {...enc3}
                    color="tertiary"
                    track={track}
                    visibilityContext={[shiftVisibilityContext]}
                />
            )}

            <KnobValue
                audioPlugin="TrackFx"
                param="VOLUME"
                label="Master Vol."
                {...enc4}
                color="tertiary"
                track={MasterTrack}
                visibilityContext={[shiftVisibilityContext]}
            />

            <Text
                text="&icon::play::filled"
                bounds={[0, ScreenHeight - 18, W1_4, 16]}
                centered={true}
                visibilityContext={[shiftVisibilityContext]}
                color={menuTextColor}
            />

            <Text
                text="Menu"
                bounds={[W1_4, ScreenHeight - 20, W1_4, 16]}
                centered={true}
                visibilityContext={[shiftVisibilityContext]}
                color={menuTextColor}
            />

            <Text
                text="Reload All"
                bounds={[W2_4, ScreenHeight - 20, W1_4, 16]}
                centered={true}
                visibilityContext={[shiftVisibilityContext]}
                color={menuTextColor}
            />

            <Text
                text="Shift"
                bounds={[W3_4, ScreenHeight - 20, W1_4, 16]}
                centered={true}
                visibilityContext={[shiftVisibilityContext]}
                color={rgb(80, 80, 80)}
            />

            <Rect
                bounds={backgroundBounds}
                color="background"
                visibilityContext={[unshiftVisibilityContext]}
            />
        </>
    );
}
