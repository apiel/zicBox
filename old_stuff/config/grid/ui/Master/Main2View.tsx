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
    enc9
} from '../constantsValue';
import { Shift } from './Shift';

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
                    <Shift />

                    <Rect
                        bounds={backgroundBounds}
                        color="background"
                        visibilityContext={[visibilityContext]}
                    />


                    <KnobValue
                        audioPlugin="FilteredFx"
                        param="CUTOFF"
                        {...enc1}
                        color="primary"
                        track={MasterTrack}
                        visibilityContext={[visibilityContext]}
                    />

                    <KnobValue
                        audioPlugin="FilteredFx"
                        param="RESONANCE"
                        {...enc2}
                        color="primary"
                        track={MasterTrack}
                        visibilityContext={[visibilityContext]}
                    />

                    <KnobValue
                        audioPlugin="FilteredFx"
                        param="FX_TYPE"
                        {...enc3}
                        color="primary"
                        track={MasterTrack}
                        visibilityContext={[visibilityContext]}
                    />

                    <KnobValue
                        audioPlugin="FilteredFx"
                        param="FX_AMOUNT"
                        {...enc4}
                        color="primary"
                        track={MasterTrack}
                        visibilityContext={[visibilityContext]}
                    />

                    <KnobValue
                        audioPlugin="FilteredFx"
                        param="FEEDBACK"
                        {...enc5}
                        color="primary"
                        track={MasterTrack}
                        visibilityContext={[visibilityContext]}
                    />

                    <KnobValue
                        audioPlugin="FilteredFx"
                        param="MIX"
                        {...enc6}
                        color="primary"
                        track={MasterTrack}
                        visibilityContext={[visibilityContext]}
                    />

                    <KnobValue
                        audioPlugin="IsolatorFx"
                        param="FREQ"
                        {...enc7}
                        color="secondary"
                        track={MasterTrack}
                        visibilityContext={[visibilityContext]}
                    />

                    <KnobValue
                        audioPlugin="IsolatorFx"
                        param="RANGE"
                        {...enc8}
                        color="secondary"
                        track={MasterTrack}
                        visibilityContext={[visibilityContext]}
                    />

                    <KnobValue
                        audioPlugin="IsolatorFx"
                        param="FX_TYPE"
                        {...enc9}
                        color="secondary"
                        track={MasterTrack}
                        visibilityContext={[visibilityContext]}
                    />

                    <KnobValue
                        audioPlugin="IsolatorFx"
                        param="FX_AMOUNT"
                        {...enc10}
                        color="secondary"
                        track={MasterTrack}
                        visibilityContext={[visibilityContext]}
                    />

                    <KnobValue
                        audioPlugin="IsolatorFx"
                        param="MIX"
                        {...enc11}
                        color="secondary"
                        track={MasterTrack}
                        visibilityContext={[visibilityContext]}
                    />

                    <KnobValue
                        audioPlugin="IsolatorFx"
                        param="ENV_SENS"
                        {...enc12}
                        color="secondary"
                        track={MasterTrack}
                        visibilityContext={[visibilityContext]}
                    />

                    <KeysTracks viewName={name} />
                </>
            }
        />
    );
}
