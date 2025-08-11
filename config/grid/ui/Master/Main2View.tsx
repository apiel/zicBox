import * as React from '@/libs/react';

import { VisibilityContext } from '@/libs/nativeComponents/component';
import { KnobValue } from '@/libs/nativeComponents/KnobValue';
import { Rect } from '@/libs/nativeComponents/Rect';
import { KeysTracks } from '../components/KeysTracks';
import { Layout } from '../components/Layout';
import { ColorTrackMaster, MasterTrack, shiftContext } from '../constants';
import {
    backgroundBounds,
    enc1,
    enc10,
    enc11,
    enc12,
    enc2,
    enc3,
    enc4,
    enc5,
    enc6,
    enc7,
    enc8,
    enc9,
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

                    {/* <KnobValue
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
                    /> */}

                    <KnobValue
                        audioPlugin="IsolatorFx"
                        param="FREQ"
                        {...enc1}
                        color="secondary"
                        track={MasterTrack}
                        visibilityContext={[visibilityContext]}
                    />

                    <KnobValue
                        audioPlugin="IsolatorFx"
                        param="RANGE"
                        {...enc2}
                        color="secondary"
                        track={MasterTrack}
                        visibilityContext={[visibilityContext]}
                    />

                    <KnobValue
                        audioPlugin="IsolatorFx"
                        param="FX_TYPE"
                        {...enc3}
                        color="secondary"
                        track={MasterTrack}
                        visibilityContext={[visibilityContext]}
                    />

                    <KnobValue
                        audioPlugin="IsolatorFx"
                        param="FX_AMOUNT"
                        {...enc4}
                        color="secondary"
                        track={MasterTrack}
                        visibilityContext={[visibilityContext]}
                    />

                    <KnobValue
                        audioPlugin="IsolatorFx"
                        param="MIX"
                        {...enc5}
                        color="secondary"
                        track={MasterTrack}
                        visibilityContext={[visibilityContext]}
                    />

                    <KnobValue
                        audioPlugin="FilterBank"
                        param="RANGE"
                        {...enc6}
                        color="secondary"
                        track={MasterTrack}
                        visibilityContext={[visibilityContext]}
                    />

                    <KnobValue
                        audioPlugin="FilteredFx"
                        param="CUTOFF"
                        {...enc7}
                        color="primary"
                        track={MasterTrack}
                        visibilityContext={[visibilityContext]}
                    />

                    <KnobValue
                        audioPlugin="FilteredFx"
                        param="RESONANCE"
                        {...enc8}
                        color="primary"
                        track={MasterTrack}
                        visibilityContext={[visibilityContext]}
                    />

                    <KnobValue
                        audioPlugin="FilteredFx"
                        param="FX_TYPE"
                        {...enc9}
                        color="primary"
                        track={MasterTrack}
                        visibilityContext={[visibilityContext]}
                    />

                    <KnobValue
                        audioPlugin="FilteredFx"
                        param="FX_AMOUNT"
                        {...enc10}
                        color="primary"
                        track={MasterTrack}
                        visibilityContext={[visibilityContext]}
                    />

                    <KnobValue
                        audioPlugin="FilteredFx"
                        param="FEEDBACK"
                        {...enc11}
                        color="primary"
                        track={MasterTrack}
                        visibilityContext={[visibilityContext]}
                    />

                    <KnobValue
                        audioPlugin="FilteredFx"
                        param="MIX"
                        {...enc12}
                        color="primary"
                        track={MasterTrack}
                        visibilityContext={[visibilityContext]}
                    />

                    <KeysTracks viewName={name} />
                </>
            }
        />
    );
}
