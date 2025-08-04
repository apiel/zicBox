import * as React from '@/libs/react';

import { VisibilityContext } from '@/libs/nativeComponents/component';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { Rect } from '@/libs/nativeComponents/Rect';
import { KeysTracks } from '../components/KeysTracks';
import { Layout } from '../components/Layout';
import {
    ColorTrackMaster,
    MasterTrack,
    shiftContext
} from '../constants';
import {
    backgroundBounds,
    enc1,
    enc2,
    enc3,
    enc4
} from '../constantsValue';
import { MasterCommon } from './Common';

export function Main2View({ name }: { name: string }) {
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
                    <MasterCommon />

                    <Rect
                        bounds={backgroundBounds}
                        color="background"
                        visibilityContext={[visibilityContext]}
                    />

                    <KnobValue
                        audioPlugin="TrackFx"
                        param="FX_TYPE"
                        {...enc1}
                        color="secondary"
                        track={MasterTrack}
                        visibilityContext={[visibilityContext]}
                    />
                    <KnobValue
                        audioPlugin="TrackFx"
                        param="FX_AMOUNT"
                        {...enc2}
                        color="secondary"
                        track={MasterTrack}
                        visibilityContext={[visibilityContext]}
                    />
                    <KnobValue
                        audioPlugin="TrackFx2"
                        param="FX_TYPE"
                        label="FX2 type"
                        {...enc3}
                        color="quaternary"
                        track={MasterTrack}
                        visibilityContext={[visibilityContext]}
                    />
                    <KnobValue
                        audioPlugin="TrackFx2"
                        param="FX_AMOUNT"
                        label="FX2 edit"
                        {...enc4}
                        color="quaternary"
                        track={MasterTrack}
                        visibilityContext={[visibilityContext]}
                    />

                    <KeysTracks viewName={name} />
                </>
            }
        />
    );
}
