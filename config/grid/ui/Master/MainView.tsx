import * as React from '@/libs/react';

import { VisibilityContext } from '@/libs/nativeComponents/component';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { Rect } from '@/libs/nativeComponents/Rect';
import { Layout } from '../components/Layout';
import { ColorTrackMaster, MasterTrack, shiftContext } from '../constants';
import { backgroundBounds, enc1 } from '../constantsValue';

export function MainView({ name }: { name: string; }) {
    const visibilityContext: VisibilityContext = {
        condition: 'SHOW_WHEN',
        index: shiftContext,
        value: 0,
    };

    return (
        <Layout
            viewName={name}
            color={ColorTrackMaster}
            title="Master"
            content={
                <>
                    <Rect
                        bounds={backgroundBounds}
                        color="background"
                        visibilityContext={[
                            { condition: 'SHOW_WHEN', index: shiftContext, value: 1 },
                        ]}
                    />
                    <KnobValue
                        audioPlugin="Tempo"
                        param="BPM"
                        {...enc1}
                        color="secondary"
                        track={MasterTrack}
                        visibilityContext={[
                            { condition: 'SHOW_WHEN', index: shiftContext, value: 1 },
                        ]}
                    />

                    {/* <KnobValue
                        audioPlugin={synthName}
                        param="DURATION"
                        {...enc1}
                        color="secondary"
                        track={track}
                        visibilityContext={[visibilityContext]}
                    /> */}

                    {/* <KeysTracks synthName={synthName} viewName={name} /> */}
                </>
            }
        />
    );
}
