import * as React from '@/libs/react';

import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { ShiftLayout, shiftVisibilityContext } from '../components/ShiftLayout';
import { MasterTrack } from '../constants';
import { enc2 } from '../constantsValue';

export function Shift() {
    return (
        <ShiftLayout track={MasterTrack} label="Master volume"
            content={
                <KnobValue
                    audioPlugin="Tempo"
                    param="BPM"
                    {...enc2}
                    color="secondary"
                    track={MasterTrack}
                    visibilityContext={[shiftVisibilityContext]}
                />
            }
        />
    );
}
