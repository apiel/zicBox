import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { Rect } from '@/libs/nativeComponents/Rect';
import {
    MasterTrack,
    shiftContext
} from '../constants';
import {
    backgroundBounds,
    enc1,
    enc2
} from '../constantsValue';

export function MasterCommon() {
    return (
        <>
            <Rect
                bounds={backgroundBounds}
                color="background"
                visibilityContext={[{ condition: 'SHOW_WHEN', index: shiftContext, value: 1 }]}
            />
            <KnobValue
                audioPlugin="Tempo"
                param="BPM"
                {...enc1}
                color="secondary"
                track={MasterTrack}
                visibilityContext={[{ condition: 'SHOW_WHEN', index: shiftContext, value: 1 }]}
            />

            <KnobValue
                audioPlugin="TrackFx2"
                param="VOLUME"
                label="Master volume"
                {...enc2}
                color="primary"
                track={MasterTrack}
                visibilityContext={[{ condition: 'SHOW_WHEN', index: shiftContext, value: 1 }]}
            />
        </>
    );
}
